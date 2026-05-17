#ifndef RENDERER_H
#define RENDERER_H

typedef struct render_context_t render_context_t;

int render_init_context(render_context_t* a_context, uint16_t a_w, uint16_t a_h, uint16_t a_mhz);
int render_set_draw_area(render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h);
int render_row_pixels(render_context_t* a_context, uint16_t a_scl_len);
int render_8x16numbers(render_context_t* a_context, const char a_char, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y);
int render_fill_area(render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color);

#endif // RENDERER_H
