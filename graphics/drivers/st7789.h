#ifndef ST7789_H
#define ST7789_H

typedef struct render_context_t render_context_t;

ERROR_TYPE_T st7789_init(render_context_t* a_ctx, u16 a_w, u16 a_h, u16 a_x_offset, u16 a_y_offset, u8 a_screen_index);
void st7789_draw_rect(render_context_t* a_ctx, u16 a_x, u16 a_y, u16 a_w, u16 a_h, u16 a_color);
void st7789_draw_8x16glyphs(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
void st7789_draw_4x8glyphs(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y);
void st7789_flush(render_context_t* a_ctx);

#endif // ST7789_H
