#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "hardware/spi.h"

#define ST7789_MAX_WIDTH 320

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
            //i2c_inst_t* i2c;
        } i2c;
    };

    uint8_t row_buf[ST7789_MAX_WIDTH * 2]; // used size = width * pixel_size


} render_context_t;

#endif // RENDER_TYPES_H
