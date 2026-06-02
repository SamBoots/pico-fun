#include <stdlib.h>
#include "pico/stdlib.h"
#include "render_types.h"
#include "renderer.h"
#include "drivers/st7789.h"
#include "drivers/ssd1306.h"
#include "font8x16.h"
#include "font4x8.h"
#include "string.h"

int render_load_func(render_context_t* a_ctx, SCREEN_DRIVER a_driver)
{
    switch (a_driver)
    {
    case DRIVER_SSD1306:
        a_ctx->init = ssd1306_init;
        a_ctx->draw_pixel = ssd1306_draw_pixel;
        a_ctx->draw_rect = ssd1306_draw_rect;
        a_ctx->draw_8x16glyphs = ssd1306_draw_8x16glyphs;
        a_ctx->draw_4x8glyphs = ssd1306_draw_4x8glyphs;
        a_ctx->flush = ssd1306_flush;
        break;
    case DRIVER_ST7789:
        a_ctx->init = st7789_init;
        a_ctx->draw_pixel = st7789_draw_pixel;
        a_ctx->draw_rect = st7789_draw_rect;
        a_ctx->draw_8x16glyphs = st7789_draw_8x16glyphs;
        a_ctx->draw_4x8glyphs = st7789_draw_4x8glyphs;
        a_ctx->flush = st7789_flush;
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

int render_init_context(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_x_offset, uint16_t a_y_offset, uint16_t a_mhz)
{
    a_ctx->init(a_ctx, a_w, a_h, a_x_offset, a_y_offset, a_mhz);
    return 1;
}

int render_draw_pixel(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_color)
{
    a_ctx->draw_pixel(a_ctx, a_x, a_y, a_color);
    return 1;
}

int render_8x16glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    int glyph_w = 8 * a_scale;
    int glyph_h = 16 * a_scale;
    for (int i = 0; i < a_len; i++)
    {
        const uint8_t* glyph = font8x16_glyph(a_str[i]);
        int x_offset = i * (glyph_w + a_spacing);
        for (int y = 0; y < glyph_h; y++)
        {
            uint8_t bits = glyph[y / a_scale];
            for (int x = 0; x < glyph_w; x++)
            {
                uint16_t color = (bits & (1 << (7 - x / a_scale))) ? a_color : 0x0000;
                a_ctx->draw_pixel(a_ctx, a_x + x + x_offset, a_y + y, color);
            }
        }
    }
    return 1;
}

int render_4x8glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    int glyph_w = 4 * a_scale;
    int glyph_h = 8 * a_scale;
    for (int i = 0; i < a_len; i++)
    {
        const uint8_t* glyph = font4x8_glyph(a_str[i]);
        int x_offset = i * (glyph_w + a_spacing);
        for (int x = 0; x < glyph_w; x++)
        {
            uint8_t bits = glyph[x / a_scale];
            for (int y  = 0; y < glyph_h; y++)
            {
                uint16_t color = (bits & (1 << (7 - y / a_scale))) ? a_color : 0x0000;
                a_ctx->draw_pixel(a_ctx, a_x + x + x_offset, a_y + y, color);
            }
        }
    }
    return 1;
}

int render_8x16glyphs_2x2border(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    uint16_t padding = 2 * a_scale;

    uint16_t glyph_w = 8 * a_scale;
    uint16_t glyph_h = 16 * a_scale;

    uint16_t text_w = glyph_w * a_len + ((a_len - 1) * a_spacing * a_scale);
    uint16_t text_h = glyph_h;

    uint16_t border_x0 = a_x;
    uint16_t border_y0 = a_y;

    uint16_t border_x1 = a_x + text_w + padding + padding - 1;
    uint16_t border_y1 = a_y + text_h + padding + padding - 1;

    render_8x16glyphs(a_ctx, a_str, a_len, a_spacing, a_scale, a_color, a_x + padding, a_y + padding);
    // now draw border
    
    for (int x = border_x0; x < border_x1; x++)
    {
        a_ctx->draw_pixel(a_ctx, x, border_y0, a_color);
        a_ctx->draw_pixel(a_ctx, x, border_y1, a_color);
    }
    for (int y = border_y0; y < border_y1; y++)
    {
        a_ctx->draw_pixel(a_ctx, border_x0, y, a_color);
        a_ctx->draw_pixel(a_ctx, border_x1, y, a_color);
    }

    return 1;
}

int render_4x8glyphs_2x2border(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    uint16_t padding = 2 * a_scale;

    uint16_t glyph_w = 4 * a_scale;
    uint16_t glyph_h = 8 * a_scale;

    uint16_t text_w = glyph_w * a_len + ((a_len - 1) * a_spacing * a_scale);
    uint16_t text_h = glyph_h;

    uint16_t border_x0 = a_x;
    uint16_t border_y0 = a_y;

    uint16_t border_x1 = a_x + text_w + padding + padding - 1;
    uint16_t border_y1 = a_y + text_h + padding + padding - 1;

    a_ctx->draw_4x8glyphs(a_ctx, a_str, a_len, a_spacing, a_scale, a_color, a_x + padding, a_y + padding);
    // now draw border
    
    for (int x = border_x0; x < border_x1; x++)
    {
        a_ctx->draw_pixel(a_ctx, x, border_y0, a_color);
        a_ctx->draw_pixel(a_ctx, x, border_y1, a_color);
    }
    for (int y = border_y0; y < border_y1; y++)
    {
        a_ctx->draw_pixel(a_ctx, border_x0, y, a_color);
        a_ctx->draw_pixel(a_ctx, border_x1, y, a_color);
    }

    return 1;
}

int render_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color)
{
    a_ctx->draw_rect(a_ctx, a_x, a_y, a_w, a_h, a_color);
    return 1;
}

int render_fill(render_context_t* a_ctx, uint16_t a_color)
{
    a_ctx->draw_rect(a_ctx, 0, 0, a_ctx->width, a_ctx->height, a_color);
    return 1;
}

int render_flush(render_context_t* a_ctx)
{
    a_ctx->flush(a_ctx);
    memset(a_ctx->buffer, 0, sizeof(a_ctx->buffer));
    return 1;
}
