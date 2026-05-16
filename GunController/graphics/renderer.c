#include <stdlib.h>
#include "pico/stdlib.h"
#include "renderer.h"
#include "../screendrivers/st7789.h"
#include "../types.h"
#include "font8x16.h"

#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

int set_draw_area(render_context_t* a_context, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h)
{
    st7789_set_cursor(a_context, a_x, a_y, a_w, a_h);
    return 1;
}

int draw_scl_pixels(render_context_t* a_context, uint16_t a_scl_len)
{
    if (a_scl_len > sizeof(a_context->scl_buffer))
    {
        return 0;
    }
    st7789_write(a_context, a_context->scl_buffer, a_scl_len);
    return 1;
}

int draw_8x16numbers(render_context_t* a_context, const char a_char, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    int draw_width = 8 * a_scale;
    int draw_height = 16 * a_scale;

    const uint8_t* glyph = font8x16[a_char - FONT_FIRST_CHAR];
    if (set_draw_area(a_context, a_x, a_y, a_x + draw_width, a_y + draw_height) == 0)
        return 0;
    // do numbers
    for (int y = 0; y < draw_height; y++)
    {
        uint8_t bits = glyph[y / a_scale];
        for (int x = 0; x < draw_width; x++)
        {
            uint16_t color = (bits & (1 << (7 - x / a_scale))) ? a_color : 0x0000;
            a_context->scl_buffer[x * 2] = color >> 8;
            a_context->scl_buffer[x * 2 + 1] = color & 0xFF;
        }
        if (draw_scl_pixels(a_context, draw_width) == 0)
            return 0;

    }

    return 1;
}
