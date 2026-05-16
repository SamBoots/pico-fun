#include "hardware/spi.h"

#define SPI_PORT    spi0

void st7789_caset(uint16_t a_xs, uint16_t a_xe);
void st7789_raset(uint16_t a_ys, uint16_t a_ye);
void st7789_init(const spi_inst_t* a_spi, uint16_t a_w, uint16_t a_h, uint16_t a_mhz);
void st7789_set_cursor(uint16_t a_x, uint16_t a_y, uint16_t a_spi_w, uint16_t a_spi_h);
void st7789_ramwr();
void st7789_write(const void* a_data, size_t a_len);
void st7789_put(uint16_t a_pixel);
void st7789_fill(uint16_t a_pixel, uint16_t a_spi_w, uint16_t a_spi_h);
