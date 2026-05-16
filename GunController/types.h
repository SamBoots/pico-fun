#ifndef TYPES_H
#define TYPES_H

#include "hardware/spi.h"  // spi_inst_t is defined here

typedef struct render_context_t
{
    uint16_t width;
    uint16_t height;
    uint16_t pixel_size;
    uint16_t mhz;

    uint pin_clk;
    uint pin_din;
    uint pin_dc;
    uint pin_cs;
    uint pin_rst;
    uint pin_bl;

    spi_inst_t* spi;
} render_context_t;

#endif // TYPES_H
