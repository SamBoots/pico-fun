#include <stdlib.h>
#include "pico/stdlib.h"
#include "../render_types.h"
#include "st7789.h"
#include "../font8x16.h"
#include "../font4x8.h"
#include "string.h"

// STANDARDIZE THESE HELPERS

static uint8_t* get_buffer_and_swap(render_context_t* a_ctx)
{
    uint8_t* buf = a_ctx->buffer[a_ctx->buffer_index];
    a_ctx->buffer_index ^= 1;
    return buf;
}
 
static inline void dc_low(const render_context_t* a_ctx)  { gpio_put(a_ctx->spi.pin_dc, 0); }
static inline void dc_high(const render_context_t* a_ctx) { gpio_put(a_ctx->spi.pin_dc, 1); }
static inline void cs_low(const render_context_t* a_ctx)  { gpio_put(a_ctx->spi.pin_cs, 0); }
static inline void cs_high(const render_context_t* a_ctx) { gpio_put(a_ctx->spi.pin_cs, 1); }

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
        (a_xs + a_ctx->x_offset) >> 8,
        (a_xs + a_ctx->x_offset) & 0xff,
        (a_xe + a_ctx->x_offset) >> 8,
        (a_xe + a_ctx->x_offset) & 0xff
    };
    st7789_send_cmd(a_ctx, 0x2a);
    st7789_send_data(a_ctx, data, sizeof(data));
}

static void st7789_raset(const render_context_t* a_ctx, uint16_t a_ys, uint16_t a_ye)
{
    const uint8_t data[] = {
        (a_ys + a_ctx->y_offset) >> 8,
        (a_ys + a_ctx->y_offset) & 0xff,
        (a_ye + a_ctx->y_offset) >> 8,
        (a_ye + a_ctx->y_offset) & 0xff
    };
    st7789_send_cmd(a_ctx, 0x2b);
    st7789_send_data(a_ctx, data, sizeof(data));
}

static void st7789_flush_no_clear(render_context_t* a_ctx)
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
}

void st7789_init(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_x_offset, uint16_t a_y_offset, uint16_t a_mhz)
{
    a_ctx->width = a_w;
    a_ctx->height = a_h;
    a_ctx->x_offset = a_x_offset;
    a_ctx->y_offset = a_y_offset;
    a_ctx->pixel_size = 16; // in bits
    a_ctx->mhz = a_mhz;

    a_ctx->spi.pin_clk = 18;
    a_ctx->spi.pin_din = 19;
    a_ctx->spi.pin_dc = 20;
    a_ctx->spi.pin_cs = 17;
    a_ctx->spi.pin_rst = 21;
    a_ctx->spi.pin_bl = 22;
    a_ctx->spi.spi = spi0; // just do SPI0 who cares
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

    st7789_send_cmd(a_ctx, 0x36);                     // MADCTL
    st7789_send_data(a_ctx, (uint8_t[]){ 0x00 }, 1);  // normal orientation
    sleep_ms(10);

    st7789_send_cmd(a_ctx, 0x21); sleep_ms(150);  // INVON
    st7789_send_cmd(a_ctx, 0x29); sleep_ms(200);  // DISPON

    gpio_put(a_ctx->spi.pin_bl, 1);           // backlight on
    st7789_draw_rect(a_ctx, 0, 0, a_ctx->width, a_ctx->height, 0);

    a_ctx->dma_chan = dma_claim_unused_channel(true);
}

static void begin_draw(render_context_t* a_ctx, uint16_t a_x0, uint16_t a_x1, uint16_t a_y0, uint16_t a_y1)
{
    st7789_caset(a_ctx, a_x0, a_x1);
    st7789_raset(a_ctx, a_y0, a_y1);
    cs_low(a_ctx);
    dc_low(a_ctx);
    st7789_send_cmd(a_ctx, 0x2C);
    dc_high(a_ctx)
}

static void end_draw(render_context_t* a_ctx)
{
    cs_high(a_ctx);
}

static void st7789_stream_wait_idle(render_context_t* a_ctx)
{
    while (dma_channel_is_busy(a_ctx->dma_chan)) tight_loop_contents();
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

static void draw_band(render_context_t* a_ctx, uint16_t a_w, uint16_t a_rows, const uint8_t* a_buf)
{
    st7789_stream_wait_idle(a_ctx);

    // setup DMA
    dma_channel_config dma_conf = dma_channel_get_default_config(a_ctx->dma_chan);
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_8);
    channel_config_set_dreq(&dma_conf, spi_get_dreq(a_ctx->spi.spi, true));
    channel_config_set_read_increment(&dma_conf, true);
    channel_config_set_write_increment(&dma_conf, false);

    dma_channel_configure(
        a_ctx->dma_chan, 
        &dma_conf,
        &spi_get_hw(a_ctx->spi.spi)->dr,
        a_buf,
        (uint32_t)a_rows * a_w * 2u,
        true);
}

void st7789_draw_rect(render_context_t* a_ctx, uint16_t a_x0, uint16_t a_x1, uint16_t a_y0, uint16_t a_y1, uint16_t a_color)
{
    uint16_t width  = a_x1 - a_x0 + 1;
    uint16_t height = a_y1 - a_y0 + 1;

    begin_draw(a_ctx, a_x0, a_x1, a_y0, a_y1);
    for (size_t row = 0; row < a_y1 - a_y0; row += BAND_ROWS)
    {
        uint16_t current_rows = (height - row < BAND_ROWS) ? (height - row) : BAND_ROWS;
        
        uint8_t* buf = get_buffer_and_swap(a_ctx);
        for (uint32_t i = 0; i < width * current_rows; i++)
        {
            buf[i * 2]     = a_color >> 8;
            buf[i * 2 + 1] = a_color & 0xFF;
        }
        draw_band(a_ctx, width, current_rows, buf);
    }
    end_draw(a_ctx);
}

void st7789_draw_8x16glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y)
{
    int glyph_w = 8 * a_scale;
    int glyph_h = 16 * a_scale;
    for (int i = 0; i < a_len; i++)
    {
        const uint8_t* glyph = font8x16_glyph(a_str[i]);
        int x_offset = i * (glyph_w + a_spacing);
        draw_lines_horizontal(a_ctx, a_x + x_offset - a_spacing, a_y, a_spacing, glyph_h, a_back_color);
        for (int y = 0; y < glyph_h; y++)
        {
            uint8_t bits = glyph[y / a_scale];
            for (int x = 0; x < glyph_w; x++)
            {
                uint16_t color = (bits & (1 << (7 - x / a_scale))) ? a_front_color : a_back_color;
                st7789_draw_pixel(a_ctx, a_x + x + x_offset, a_y + y, color);
            }
        }
    }
}

void st7789_draw_4x8glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y)
{
    int glyph_w = 4 * a_scale;
    int glyph_h = 8 * a_scale;
    for (int i = 0; i < a_len; i++)
    {
        const uint8_t* glyph = font4x8_glyph(a_str[i]);
        int x_offset = i * (glyph_w + a_spacing);
        draw_lines_horizontal(a_ctx, a_x + x_offset - a_spacing, a_y, a_spacing, glyph_h, a_back_color);
        for (int x = 0; x < glyph_w; x++)
        {
            uint8_t bits = glyph[x / a_scale];
            for (int y  = 0; y < glyph_h; y++)
            {
                uint16_t color = (bits & (1 << (7 - y / a_scale))) ? a_front_color : a_back_color;
                st7789_draw_pixel(a_ctx, a_x + x + x_offset, a_y + y, color);
            }
        }
    }
}
