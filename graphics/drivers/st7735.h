#ifndef ST7735_H
#define ST7735_H
 
typedef struct render_context_t render_context_t;

void st7735_init(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_mhz);
void st7735_draw_pixel(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_color);
void st7735_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color);
void st7735_draw_8x16glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y);
void st7735_draw_4x8glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y);
void st7735_flush(render_context_t* a_ctx);

#endif // ST7735_H
