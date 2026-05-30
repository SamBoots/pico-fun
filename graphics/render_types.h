#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "hardware/spi.h"
#include "hardware/i2c.h"

#define ST7789_MAX_WIDTH 320
#define SSD1306_MAX_FRAMEBUFFER 1024


#define ROW_BUFFER_SIZE SSD1306_MAX_FRAMEBUFFER // resize this if another driver has a bigger buffer

typedef struct render_context_t
{
    uint16_t width;
    uint16_t height;
    uint16_t pixel_size;
    uint16_t mhz;

    union 
    {
        struct
        {
            spi_inst_t* spi;
            uint pin_clk;
            uint pin_din;
            uint pin_cs;
            uint pin_dc;
            uint pin_rst;
            uint pin_bl;
        } spi;

        struct 
        {
            uint pin_sda;
            uint pin_scl;
            uint8_t address;
            uint16_t col_start, col_end;
            uint16_t row_start, row_end;
            i2c_inst_t* i2c;
        } i2c;
    };

    uint8_t buffer[ROW_BUFFER_SIZE];
    uint16_t buffer_offset;

} render_context_t;

#endif // RENDER_TYPES_H
