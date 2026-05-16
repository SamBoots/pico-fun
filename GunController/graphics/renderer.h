#ifndef RENDERER_H
#define RENDERER_H

typedef struct render_context_t render_context_t;

int set_draw_area(render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h);
int draw_scl_pixels(render_context_t* a_context, uint16_t a_scl_len);
int draw_8x16numbers(render_context_t* a_context, const char a_char, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y);
#endif // RENDERER_H
