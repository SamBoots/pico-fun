#include <stdlib.h>
#include "pico/stdlib.h"
#include "../types.h"
#include "renderer.h"
#include "drivers/st7789.h"
#include "font8x16.h"

int render_init_context(render_context_t* a_context, uint16_t a_w, uint16_t a_h, uint16_t a_mhz)
{
    st7789_init(a_context, a_w, a_h, a_mhz);
    return 1;
}

int render_set_draw_area(render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h)
{
    st7789_set_cursor(a_context, a_x, a_y, a_w, a_h);
    return 1;
}

int render_row_pixels(render_context_t* a_context, uint16_t a_scl_len)
{
    if (a_scl_len > sizeof(a_context->row_buf))
    {
        return 0;
    }
    st7789_write(a_context, a_context->row_buf, a_scl_len);
    return 1;
}

int render_8x16numbers(render_context_t* a_context, const char a_char, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    int draw_width = 8 * a_scale;
    int draw_height = 16 * a_scale;

    const uint8_t* glyph = font8x16[a_char - FONT_FIRST_CHAR];
    if (render_set_draw_area(a_context, a_x, a_y, a_x + draw_width, a_y + draw_height) == 0)
        return 0;
    // do numbers
    for (int y = 0; y < draw_height; y++)
    {
        uint8_t bits = glyph[y / a_scale];
        for (int x = 0; x < draw_width; x++)
        {
            uint16_t color = (bits & (1 << (7 - x / a_scale))) ? a_color : 0x0000;
            a_context->row_buf[x * 2] = color >> 8;
            a_context->row_buf[x * 2 + 1] = color & 0xFF;
        }
        if (render_row_pixels(a_context, draw_width) == 0)
            return 0;

    }

    return 1;
}

int render_fill_area(render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color)
{
    if (render_set_draw_area(a_context, a_x, a_y, a_x + a_w, a_y + a_h) == 0)
        return 0;

    for (int x = 0; x < a_w; x++)
        a_context->row_buf[x] = a_color;

    for (int y = 0; y < a_h; y++)
        render_row_pixels(a_context, a_w);
}
