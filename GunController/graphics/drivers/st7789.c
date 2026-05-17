#include <stdlib.h>
#include "pico/stdlib.h"
#include "../../types.h"
#include "st7789.h"
 
static inline void dc_low(const render_context_t* a_context)  { gpio_put(a_context->pin_dc, 0); }
static inline void dc_high(const render_context_t* a_context) { gpio_put(a_context->pin_dc, 1); }
static inline void cs_low(const render_context_t* a_context)  { gpio_put(a_context->pin_cs, 0); }
static inline void cs_high(const render_context_t* a_context) { gpio_put(a_context->pin_cs, 1); }

// :)
static bool st7789_data_mode = false;
 
static void st7789_cmd(const render_context_t* a_context, uint8_t a_cmd, const uint8_t* a_data, size_t a_len)
{
    spi_set_format(a_context->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    st7789_data_mode = false;

    sleep_us(1);
    cs_low(a_context);
    dc_low(a_context);
    sleep_us(1);
    
    spi_write_blocking(a_context->spi, &a_cmd, sizeof(a_cmd));

    if (a_len)
    {
        sleep_us(1);
        dc_high(a_context);
        sleep_us(1);
    
        spi_write_blocking(a_context->spi, a_data, a_len);
    }
    sleep_us(1);
    cs_high(a_context);
    dc_high(a_context);
    sleep_us(1);
}

void st7789_caset(const render_context_t* a_context, uint16_t a_xs, uint16_t a_xe)
{
    const uint8_t data[] = {
        a_xs >> 8,
        a_xs & 0xff,
        a_xe >> 8,
        a_xe & 0xff,
    };

    // CASET (2Ah): Column Address Set
    st7789_cmd(a_context, 0x2a, data, sizeof(data));
}

void st7789_raset(const render_context_t* a_context, uint16_t a_ys, uint16_t a_ye)
{
    const uint8_t data[] = {
        a_ys >> 8,
        a_ys & 0xff,
        a_ye >> 8,
        a_ye & 0xff,
    };

    // RASET (2Bh): Row Address Set
    st7789_cmd(a_context, 0x2b, data, sizeof(data));
}

static void st7789_reg(const render_context_t* a_context)
{
    spi_init(a_context->spi, a_context->mhz * 1000 * 1000);
    spi_set_format(a_context->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(a_context->pin_clk, GPIO_FUNC_SPI);
    gpio_set_function(a_context->pin_din, GPIO_FUNC_SPI);

    gpio_init(a_context->pin_cs);  gpio_set_dir(a_context->pin_cs,  GPIO_OUT); gpio_put(a_context->pin_cs,  1);
    gpio_init(a_context->pin_dc);  gpio_set_dir(a_context->pin_dc,  GPIO_OUT); gpio_put(a_context->pin_dc, 1);
    gpio_init(a_context->pin_rst); gpio_set_dir(a_context->pin_rst, GPIO_OUT); gpio_put(a_context->pin_rst, 1);
    gpio_init(a_context->pin_bl);  gpio_set_dir(a_context->pin_bl,  GPIO_OUT);

    sleep_ms(100);
    // SWRESET (01h): Software Reset
    st7789_cmd(a_context, 0x01, NULL, 0);
    sleep_ms(150);

    // SLPOUT (11h): Sleep Out
    st7789_cmd(a_context, 0x11, NULL, 0);
    sleep_ms(50); 

    // COLMOD (3Ah): Interface Pixel Format
    // - RGB interface color format     = 65K of RGB interface
    // - Control interface color format = 16bit/pixel
    st7789_cmd(a_context, 0x3a, (uint8_t[]){ 0x55 }, 1);
    sleep_ms(10);

    // MADCTL (36h): Memory Data Access Control
    // - Page Address Order            = Top to Bottom
    // - Column Address Order          = Left to Right
    // - Page/Column Order             = Normal Mode
    // - Line Address Order            = LCD Refresh Top to Bottom
    // - RGB/BGR Order                 = RGB
    // - Display Data Latch Data Order = LCD Refresh Left to Right
    st7789_cmd(a_context, 0x36, (uint8_t[]){ 0x00 }, 1);

    st7789_caset(a_context, 0, a_context->width - 1);
    st7789_raset(a_context, 0, a_context->height - 1);

    // INVON (21h): Display Inversion On
    st7789_cmd(a_context, 0x21, NULL, 0);
    sleep_ms(10);

    // NORON (13h): Normal Display Mode On
    st7789_cmd(a_context, 0x13, NULL, 0);
    sleep_ms(10);

    // DISPON (29h): Display On
    st7789_cmd(a_context, 0x29, NULL, 0);
    sleep_ms(10);

    gpio_put(a_context->pin_bl, 1);
}
 
void st7789_init(render_context_t* a_context, uint16_t a_w, uint16_t a_h, uint16_t a_mhz)
{
    // Temp init for cables, should be universal for one screen?
    a_context->width = a_w;
    a_context->height = a_h;
    a_context->pixel_size = 2;
    a_context->mhz = a_mhz;

    a_context->pin_clk = PICO_DEFAULT_SPI_SCK_PIN;
    a_context->pin_din = PICO_DEFAULT_SPI_TX_PIN;
    a_context->pin_dc = 20;
    a_context->pin_cs = PICO_DEFAULT_SPI_CSN_PIN;
    a_context->pin_rst = 21;
    a_context->pin_bl = 22;

    a_context->spi = spi0;
    st7789_reg(a_context);
}

void st7789_set_cursor(const render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h)
{
    st7789_caset(a_context, a_x, a_w);
    st7789_raset(a_context, a_y, a_h);
}

void st7789_ramwr(const render_context_t* a_context)
{
    sleep_us(1);
    cs_low(a_context);
    dc_low(a_context);
    sleep_us(1);

    // RAMWR (2Ch): Memory Write
    uint8_t cmd = 0x2c;
    spi_write_blocking(a_context->spi, &cmd, sizeof(cmd));

    sleep_us(1);
    cs_high(a_context);
    dc_high(a_context);
    sleep_us(1);
}

void st7789_write(const render_context_t* a_context, const void* a_data, size_t a_len)
{
    if (!st7789_data_mode) {
        st7789_ramwr(a_context);
        spi_set_format(a_context->spi, a_context->pixel_size * 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        st7789_data_mode = true;
    }

    spi_write16_blocking(a_context->spi, a_data, a_len / 2);
}

void st7789_put(const render_context_t* a_context, uint16_t a_pixel)
{
    st7789_write(a_context, &a_pixel, sizeof(a_pixel));
}

void st7789_fill(const render_context_t* a_context, uint16_t a_pixel)
{
    int num_pixels = a_context->width * a_context->height;

    st7789_set_cursor(a_context, 0, 0, a_context->width, a_context->height);

    for (int i = 0; i < num_pixels; i++) {
        st7789_put(a_context, a_pixel);
    }
}
