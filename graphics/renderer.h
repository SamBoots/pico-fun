#ifndef RENDERER_H
#define RENDERER_H

typedef struct render_context_t render_context_t;

typedef enum SCREEN_DRIVER
{
    DRIVER_ST7789
} SCREEN_DRIVER;

ERROR_TYPE_T render_load_func(render_context_t* a_ctx, SCREEN_DRIVER a_driver);
ERROR_TYPE_T render_init_context(render_context_t* a_ctx, u16 a_w, u16 a_h, u16 a_x_offset, u16 a_y_offset, u8 a_screen_index);
void render_8x16glyphs(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
void render_4x8glyphs(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
void render_8x16glyphs_2x2border(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
void render_4x8glyphs_2x2border(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
void render_draw_rect(render_context_t* a_ctx, u16 a_x0, u16 a_x1, u16 a_y0, u16 a_y1, u16 a_color);
void render_fill(render_context_t* a_ctx, u16 a_color);
void render_flush(render_context_t* a_ctx);

#endif // RENDERER_H
