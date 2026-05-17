#ifndef ST7789_H
#define ST7789_H

typedef struct render_context_t render_context_t;

void st7789_caset(const render_context_t* a_context, uint16_t a_xs, uint16_t a_xe);
void st7789_raset(const render_context_t* a_context, uint16_t a_ys, uint16_t a_ye);
void st7789_init(render_context_t* a_context, uint16_t a_w, uint16_t a_h, uint16_t a_mhz);
void st7789_ramwr(const render_context_t* a_context);
void st7789_write16(const render_context_t* a_context, const void* a_data, size_t a_len_pixels);
void st7789_write8(const render_context_t* a_context, const void* a_data, size_t a_len_pixels);

#endif // ST7789_H
