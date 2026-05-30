#include <stdlib.h>
#include "pico/stdlib.h"
#include "../render_types.h"
#include "st7789.h"
#include "../font8x16.h"
#include "../font4x8.h"
#include "string.h"
 
static inline void dc_low(const render_context_t* a_context)  { gpio_put(a_context->spi.pin_dc, 0); }
static inline void dc_high(const render_context_t* a_context) { gpio_put(a_context->spi.pin_dc, 1); }
static inline void cs_low(const render_context_t* a_context)  { gpio_put(a_context->spi.pin_cs, 0); }
static inline void cs_high(const render_context_t* a_context) { gpio_put(a_context->spi.pin_cs, 1); }

static void st7789_send_cmd(const render_context_t* a_ctx, uint8_t a_cmd)
{
    spi_set_format(a_ctx->spi.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    cs_low(a_ctx);
    dc_low(a_ctx);
    spi_write_blocking(a_ctx->spi.spi, &a_cmd, 1);
    cs_high(a_ctx);
}

static void st7789_send_data(const render_context_t* a_ctx, const uint8_t* a_data, size_t a_len)
{
    cs_low(a_ctx);
    dc_high(a_ctx);
    spi_write_blocking(a_ctx->spi.spi, a_data, a_len);
    cs_high(a_ctx);
}

static void st7789_caset(const render_context_t* a_ctx, uint16_t a_xs, uint16_t a_xe)
{
    const uint8_t data[] = {
        a_xs >> 8,
        a_xs & 0xff,
        a_xe >> 8,
        a_xe & 0xff
    };
    st7789_send_cmd(a_ctx, 0x2a);
    st7789_send_data(a_ctx, data, sizeof(data));
}

static void st7789_raset(const render_context_t* a_ctx, uint16_t a_ys, uint16_t a_ye)
{
    const uint8_t data[] = {
        a_ys >> 8,
        a_ys & 0xff,
        a_ye >> 8,
        a_ye & 0xff
    };
    st7789_send_cmd(a_ctx, 0x2b);
    st7789_send_data(a_ctx, data, sizeof(data));
}

void st7789_init(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_mhz)
{
    a_ctx->width = a_w;
    a_ctx->height = a_h;
    a_ctx->pixel_size = 8; // in bits, maybe 16
    a_ctx->mhz = a_mhz;
 
    a_ctx->spi.pin_clk = 18;
    a_ctx->spi.pin_din = 19;
    a_ctx->spi.pin_dc = 20;
    a_ctx->spi.pin_cs = 17;
    a_ctx->spi.pin_rst = 21;
    a_ctx->spi.pin_bl = 22;
    a_ctx->spi.spi = spi0; // just do SPI0 who cares
    a_ctx->buffer_offset = 0;
    memset(&a_ctx->buffer, 0, sizeof(a_ctx->buffer));

    spi_init(a_ctx->spi.spi, a_ctx->mhz * 1000 * 1000);
    spi_set_format(a_ctx->spi.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(a_ctx->spi.pin_clk, GPIO_FUNC_SPI);
    gpio_set_function(a_ctx->spi.pin_din, GPIO_FUNC_SPI);

    gpio_init(a_ctx->spi.pin_cs);  gpio_set_dir(a_ctx->spi.pin_cs,  GPIO_OUT); gpio_put(a_ctx->spi.pin_cs,  1);
    gpio_init(a_ctx->spi.pin_dc);  gpio_set_dir(a_ctx->spi.pin_dc,  GPIO_OUT); gpio_put(a_ctx->spi.pin_dc,  1);
    gpio_init(a_ctx->spi.pin_rst); gpio_set_dir(a_ctx->spi.pin_rst, GPIO_OUT); gpio_put(a_ctx->spi.pin_rst, 1);
    gpio_init(a_ctx->spi.pin_bl);  gpio_set_dir(a_ctx->spi.pin_bl,  GPIO_OUT); gpio_put(a_ctx->spi.pin_bl,  0);

    // reset
    gpio_put(a_ctx->spi.pin_rst, 0); sleep_ms(100);
    gpio_put(a_ctx->spi.pin_rst, 1); sleep_ms(100);

    // init sequence
    st7789_send_cmd(a_ctx, 0x01); sleep_ms(150); // SWRESET
    st7789_send_cmd(a_ctx, 0x11); sleep_ms(150);  // SLPOUT

    st7789_send_cmd(a_ctx, 0x3A);                // COLMOD
    st7789_send_data(a_ctx, (uint8_t[]){ 0x55 }, 1); // 16bit RGB565
    sleep_ms(10);

    st7789_send_cmd(a_ctx, 0x21); sleep_ms(150);  // INVON
    st7789_send_cmd(a_ctx, 0x29); sleep_ms(200);  // DISPON

    gpio_put(a_ctx->spi.pin_bl, 1);           // backlight on
    st7789_draw_rect(a_ctx, 0, 0, a_ctx->width, a_ctx->height, 0);
}

void st7789_draw_pixel(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_color)
{
    st7789_caset(a_ctx, a_x, a_x);
    st7789_raset(a_ctx, a_y, a_y);
    
    // send RAMWR + pixel data without releasing CS in between
    uint8_t cmd = 0x2C;
    spi_set_format(a_ctx->spi.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    cs_low(a_ctx);
    dc_low(a_ctx);
    spi_write_blocking(a_ctx->spi.spi, &cmd, 1);
    dc_high(a_ctx);
    uint8_t px[2] = { a_color >> 8, a_color & 0xFF };
    spi_write_blocking(a_ctx->spi.spi, px, 2);
    cs_high(a_ctx);
}

void st7789_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color)
{
    for (int i = 0; i < a_w; i++)
    {
        a_ctx->buffer[i * 2] = a_color >> 8;
        a_ctx->buffer[i * 2 + 1] = a_color & 0xFF;
    }
    a_ctx->buffer_offset = a_w * 2;
    st7789_caset(a_ctx, a_x, a_x + a_w - 1);
    
    for (int y = 0; y < a_h; y++)
    {
        st7789_raset(a_ctx, a_y + y, a_y + y);
        st7789_flush(a_ctx);
    }
    a_ctx->buffer_offset = 0;
}

void st7789_draw_8x16glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{

}

void st7789_draw_4x8glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{

}

void st7789_flush(render_context_t* a_ctx)
{
  if (a_ctx->buffer_offset == 0) return;

    uint8_t cmd = 0x2C;
    spi_set_format(a_ctx->spi.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    cs_low(a_ctx);
    dc_low(a_ctx);
    spi_write_blocking(a_ctx->spi.spi, &cmd, 1);
    dc_high(a_ctx);
    spi_write_blocking(a_ctx->spi.spi, a_ctx->buffer, a_ctx->buffer_offset);
    cs_high(a_ctx);

    a_ctx->buffer_offset = 0;
}
