#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "hardware/spi.h"
#include "hardware/i2c.h"

#define ST7789_MAX_WIDTH 320
#define SSD1306_MAX_FRAMEBUFFER 1024

#define BAND_ROWS 16
#define BAND_BUFFER_SIZE (ST7789_MAX_WIDTH * BAND_ROWS * sizeof(u16))

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F

typedef const u8* (*glyph_lookup_fn)(char);

typedef struct render_context_t
{
    u16 width;
    u16 height;
    u16 x_offset;
    u16 y_offset;
    u16 pixel_size;
    u16 mhz;
    
    union 
    {
        struct
        {
            spi_inst_t* spi;
            u8 pin_clk;
            u8 pin_din;
            u8 pin_cs;
            u8 pin_dc;
            u8 pin_rst;
            u8 pin_bl;
        } spi;

        struct
        {
            u8 pin_sda;
            u8 pin_scl;
            u8 address;
            u16 col_start, col_end;
            u16 row_start, row_end;
            i2c_inst_t* i2c;
        } i2c;
    };

    u8 buffer[2][BAND_BUFFER_SIZE];
    u8 buffer_index;
    
    int dma_chan;

    ERROR_TYPE_T (*init)(struct render_context_t* a_ctx, u16 a_w, u16 a_h, u16 a_x_offset, u16 a_y_offset, u8 a_screen_index);
    void (*draw_rect)(struct render_context_t* a_ctx, u16 a_x, u16 a_y, u16 a_w, u16 a_h, u16 a_color);
    void (*draw_8x16glyphs)(struct render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
    void (*draw_4x8glyphs)(struct render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
    void (*flush)(struct render_context_t* a_ctx);
} render_context_t;

// helper functions

static u8* get_buffer_and_swap(render_context_t* a_ctx)
{
    u8* buf = a_ctx->buffer[a_ctx->buffer_index];
    a_ctx->buffer_index ^= 1;
    return buf;
}

#endif // RENDER_TYPES_H
