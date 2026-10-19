#include "core/types.h"
#include "render_types.h"
#include "renderer.h"
#include "drivers/st7789.h"
#include "font8x16.h"
#include "font4x8.h"
#include "string.h"

ERROR_TYPE_T render_load_func(render_context_t* a_ctx, SCREEN_DRIVER a_driver)
{
    switch (a_driver)
    {
    case DRIVER_ST7789:
        a_ctx->init = st7789_init;
        a_ctx->draw_rect = st7789_draw_rect;
        a_ctx->draw_8x16glyphs = st7789_draw_8x16glyphs;
        a_ctx->draw_4x8glyphs = st7789_draw_4x8glyphs;
        a_ctx->flush = st7789_flush;
        break;
    default:
        return ERROR_TYPE_UNSUPPORTED;
        break;
    }
    return ERROR_TYPE_OK;
}

ERROR_TYPE_T render_init_context(render_context_t* a_ctx, u16 a_w, u16 a_h, u16 a_x_offset, u16 a_y_offset, u8 a_screen_index)
{
    return a_ctx->init(a_ctx, a_w, a_h, a_x_offset, a_y_offset, a_screen_index);
}

void render_8x16glyphs(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y)
{
    a_ctx->draw_8x16glyphs(a_ctx, a_str, a_len, a_spacing, a_scale, a_front_color, a_back_color, a_x, a_y);
}

void render_4x8glyphs(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y)
{
    a_ctx->draw_4x8glyphs(a_ctx, a_str, a_len, a_spacing, a_scale, a_front_color, a_back_color, a_x, a_y);
}

void render_8x16glyphs_2x2border(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y)
{
    u16 padding = 2 * a_scale;

    u16 glyph_w = 8 * a_scale;
    u16 glyph_h = 16 * a_scale;

    u16 text_w = glyph_w * a_len + ((a_len - 1) * a_spacing * a_scale);
    u16 text_h = glyph_h;

    u16 border_x0 = a_x;
    u16 border_y0 = a_y;

    u16 border_x1 = a_x + text_w + padding + padding - 1;
    u16 border_y1 = a_y + text_h + padding + padding - 1;

    render_8x16glyphs(a_ctx, a_str, a_len, a_spacing, a_scale, a_front_color, a_back_color, a_x + padding, a_y + padding);
    // now draw border
    
    u16 border_width = border_x1 - border_x0;
    u16 border_height = border_y1 - border_y0;

    a_ctx->draw_rect(a_ctx, border_x0, border_y0, border_width, a_scale, a_front_color);
    a_ctx->draw_rect(a_ctx, border_x0, border_y0, a_scale, border_height, a_front_color);

    a_ctx->draw_rect(a_ctx, border_x0, border_y1, border_width, a_scale, a_front_color);
    a_ctx->draw_rect(a_ctx, border_x1, border_y0, a_scale, border_height, a_front_color);
}

void render_4x8glyphs_2x2border(render_context_t* a_ctx, const char* a_str, u16 a_len, u16 a_spacing, u16 a_scale, u16 a_front_color, u16 a_back_color, u16 a_x, u16 a_y)
{
    u16 padding = 2 * a_scale;

    u16 glyph_w = 4 * a_scale;
    u16 glyph_h = 8 * a_scale;

    u16 text_w = glyph_w * a_len + ((a_len - 1) * a_spacing * a_scale);
    u16 text_h = glyph_h;

    u16 border_x0 = a_x;
    u16 border_y0 = a_y;

    u16 border_x1 = a_x + text_w + padding + padding - 1;
    u16 border_y1 = a_y + text_h + padding + padding - 1;

    a_ctx->draw_4x8glyphs(a_ctx, a_str, a_len, a_spacing, a_scale, a_front_color, a_back_color, a_x + padding, a_y + padding);
    // now draw border
    
    u16 border_width = border_x1 - border_x0;
    u16 border_height = border_y1 - border_y0;

    a_ctx->draw_rect(a_ctx, border_x0, border_y0, border_width, a_scale, a_front_color);
    a_ctx->draw_rect(a_ctx, border_x0, border_y0, a_scale, border_height, a_front_color);

    a_ctx->draw_rect(a_ctx, border_x0, border_y1, border_width, a_scale, a_front_color);
    a_ctx->draw_rect(a_ctx, border_x1, border_y0, a_scale, border_height, a_front_color);
}

void render_draw_rect(render_context_t* a_ctx, u16 a_x0, u16 a_x1, u16 a_y0, u16 a_y1, u16 a_color)
{
    a_ctx->draw_rect(a_ctx, a_x0, a_x1, a_y0, a_y1, a_color);
}

void render_fill(render_context_t* a_ctx, u16 a_color)
{
    a_ctx->draw_rect(a_ctx, 0, a_ctx->width, 0, a_ctx->height, a_color);
}

void render_flush(render_context_t* a_ctx)
{
    a_ctx->flush(a_ctx);
    memset(a_ctx->buffer, 0, sizeof(a_ctx->buffer));
}
