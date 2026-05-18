#include <stdlib.h>
#include "pico/stdlib.h"
#include "render_types.h"
#include "renderer.h"
#include "drivers/st7789.h"
#include "drivers/ssd1306.h"
#include "string.h"


int render_init_context(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_mhz)
{
    ssd1306_init(a_ctx, a_w, a_h, a_mhz);
    return 1;
}

int render_draw_pixel(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_color)
{
    ssd1306_draw_pixel(a_ctx, a_x, a_y, a_color);
    return 1;
}

int render_8x16numbers(render_context_t* a_ctx, const char a_char, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    ssd1306_draw_glyph(a_ctx, a_char, a_scale, a_color, a_x, a_y);
    return 1;
}

int render_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color)
{
    ssd1306_draw_rect(a_ctx, a_x, a_y, a_w, a_h, a_color);
    return 1;
}

int render_flush(render_context_t* a_ctx)
{
    ssd1306_flush(a_ctx);
    memset(a_ctx->buffer, 0, sizeof(a_ctx->buffer));
}
