#ifndef ST7789_H
#define ST7789_H

typedef struct render_context_t render_context_t;

void st7789_init(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_x_offset, uint16_t a_y_offset, uint16_t a_mhz);
void st7789_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color);
void st7789_draw_8x16glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);
void st7789_draw_4x8glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_front_color, uint16_t a_back_color, uint16_t a_x, uint16_t a_y);

#endif // ST7789_H
