#include <stdlib.h>
#include "pico/stdlib.h"
#include "st7789.h"
 
#define PIN_CLK     PICO_DEFAULT_SPI_SCK_PIN
#define PIN_DIN     PICO_DEFAULT_SPI_TX_PIN
#define PIN_DC      20
#define PIN_CS      PICO_DEFAULT_SPI_CSN_PIN
#define PIN_RST     21
#define PIN_BL      22
 
static inline void dc_low(void)  { gpio_put(PIN_DC, 0); }
static inline void dc_high(void) { gpio_put(PIN_DC, 1); }
static inline void cs_low(void)  { gpio_put(PIN_CS, 0); }
static inline void cs_high(void) { gpio_put(PIN_CS, 1); }

// :)
static bool st7789_data_mode = false;
 
static void st7789_cmd(uint8_t a_cmd, const uint8_t* a_data, size_t a_len)
{
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    st7789_data_mode = false;

    sleep_us(1);
    cs_low();
    dc_low();
    sleep_us(1);
    
    spi_write_blocking(SPI_PORT, &a_cmd, sizeof(a_cmd));

    if (a_len)
    {
        sleep_us(1);
        dc_high();
        sleep_us(1);
    
        spi_write_blocking(SPI_PORT, a_data, a_len);
    }
    sleep_us(1);
    cs_high();
    dc_high();
    sleep_us(1);
}

void st7789_caset(uint16_t a_xs, uint16_t a_xe)
{
    const uint8_t data[] = {
        a_xs >> 8,
        a_xs & 0xff,
        a_xe >> 8,
        a_xe & 0xff,
    };

    // CASET (2Ah): Column Address Set
    st7789_cmd(0x2a, data, sizeof(data));
}

void st7789_raset(uint16_t a_ys, uint16_t a_ye)
{
    const uint8_t data[] = {
        a_ys >> 8,
        a_ys & 0xff,
        a_ye >> 8,
        a_ye & 0xff,
    };

    // RASET (2Bh): Row Address Set
    st7789_cmd(0x2b, data, sizeof(data));
}
 
void st7789_init(const spi_inst_t* a_spi, uint16_t a_w, uint16_t a_h, uint16_t a_mhz)
{
    spi_init(SPI_PORT, a_mhz * 1000 * 1000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DIN, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);  gpio_set_dir(PIN_CS,  GPIO_OUT); gpio_put(PIN_CS,  1);
    gpio_init(PIN_DC);  gpio_set_dir(PIN_DC,  GPIO_OUT); gpio_put(PIN_DC, 1);
    gpio_init(PIN_RST); gpio_set_dir(PIN_RST, GPIO_OUT); gpio_put(PIN_RST, 1);
    gpio_init(PIN_BL);  gpio_set_dir(PIN_BL,  GPIO_OUT);

    sleep_ms(100);
    // SWRESET (01h): Software Reset
    st7789_cmd(0x01, NULL, 0);
    sleep_ms(150);

    // SLPOUT (11h): Sleep Out
    st7789_cmd(0x11, NULL, 0);
    sleep_ms(50); 

    // COLMOD (3Ah): Interface Pixel Format
    // - RGB interface color format     = 65K of RGB interface
    // - Control interface color format = 16bit/pixel
    st7789_cmd(0x3a, (uint8_t[]){ 0x55 }, 1);
    sleep_ms(10);

    // MADCTL (36h): Memory Data Access Control
    // - Page Address Order            = Top to Bottom
    // - Column Address Order          = Left to Right
    // - Page/Column Order             = Normal Mode
    // - Line Address Order            = LCD Refresh Top to Bottom
    // - RGB/BGR Order                 = RGB
    // - Display Data Latch Data Order = LCD Refresh Left to Right
    st7789_cmd(0x36, (uint8_t[]){ 0x00 }, 1);

    st7789_caset(0, a_w);
    st7789_raset(0, a_h);

    // INVON (21h): Display Inversion On
    st7789_cmd(0x21, NULL, 0);
    sleep_ms(10);

    // NORON (13h): Normal Display Mode On
    st7789_cmd(0x13, NULL, 0);
    sleep_ms(10);

    // DISPON (29h): Display On
    st7789_cmd(0x29, NULL, 0);
    sleep_ms(10);

    gpio_put(PIN_BL, 1);
}

void st7789_set_cursor(uint16_t a_x, uint16_t a_y, uint16_t a_spi_w, uint16_t a_spi_h)
{
    st7789_caset(a_x, a_spi_w);
    st7789_raset(a_y, a_spi_h);
}

void st7789_ramwr(void)
{
    sleep_us(1);
    cs_low();
    dc_low();
    sleep_us(1);

    // RAMWR (2Ch): Memory Write
    uint8_t cmd = 0x2c;
    spi_write_blocking(SPI_PORT, &cmd, sizeof(cmd));

    sleep_us(1);
    cs_low();
    dc_high();
    sleep_us(1);
}

void st7789_write(const void* a_data, size_t a_len)
{
    if (!st7789_data_mode) {
        st7789_ramwr();
        spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        st7789_data_mode = true;
    }

    spi_write16_blocking(spi0, a_data, a_len / 2);
}

void st7789_put(uint16_t a_pixel)
{
    st7789_write(&a_pixel, sizeof(a_pixel));
}

void st7789_fill(uint16_t a_pixel, uint16_t a_spi_w, uint16_t a_spi_h)
{
    int num_pixels = a_spi_w * a_spi_h;

    st7789_set_cursor(0, 0, a_spi_w, a_spi_h);

    for (int i = 0; i < num_pixels; i++) {
        st7789_put(a_pixel);
    }
}