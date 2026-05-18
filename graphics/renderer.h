#ifndef RENDERER_H
#define RENDERER_H

typedef struct render_context_t render_context_t;

int render_init_context(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_mhz);
int render_8x16numbers(render_context_t* a_ctx, const char a_char, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y);
int render_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color);
int render_flush(render_context_t* a_ctx);

#endif // RENDERER_H
