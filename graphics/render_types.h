#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "hardware/spi.h"
#include "hardware/i2c.h"

#define ST7789_MAX_WIDTH 320
#define SSD1306_MAX_FRAMEBUFFER 1024

#define ROW_BUFFER_SIZE SSD1306_MAX_FRAMEBUFFER // resize this if another driver has a bigger buffer

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_ORANGE  0xFD20
#define COLOR_PURPLE  0x8010

typedef struct render_context_t
{
    uint16_t width;
    uint16_t height;
    uint16_t x_offset;
    uint16_t y_offset;
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

    void (*init)(struct render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_x_offset, uint16_t a_y_offset, uint16_t a_mhz);
    void (*draw_pixel)(struct render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_color);
    void (*draw_rect)(struct render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color);
    void (*draw_8x16glyphs)(struct render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
    void (*draw_4x8glyphs)(struct render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
    void (*flush)(struct render_context_t* a_ctx);
} render_context_t;

#endif // RENDER_TYPES_H
