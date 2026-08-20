#ifndef RENDERER_H
#define RENDERER_H

typedef struct render_context_t render_context_t;

typedef enum SCREEN_DRIVER
{
    DRIVER_SSD1306,
    DRIVER_ST7789
} SCREEN_DRIVER;

int render_load_func(render_context_t* a_ctx, SCREEN_DRIVER a_driver);
int render_init_context(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_x_offset, uint16_t a_y_offset, uint16_t a_mhz);
int render_8x16glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
int render_4x8glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
int render_8x16glyphs_2x2border(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
int render_4x8glyphs_2x2border(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
int render_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color);
int render_fill(render_context_t* a_ctx, uint16_t a_color);
int render_flush(render_context_t* a_ctx);

#endif // RENDERER_H
