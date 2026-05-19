#include <stdlib.h>
#include "pico/stdlib.h"
#include "../render_types.h"
#include "ssd1306.h"
#include "../font8x16.h"
#include "string.h"

#define SSD1306_COL_OFFSET 32  // 64px panel sits at columns 32..95 on the 128px chip

static void send_cmd(const render_context_t* a_ctx, uint8_t a_cmd)
{
    uint8_t buf[2] = { 0x00, a_cmd };
    i2c_write_blocking(i2c0, a_ctx->i2c.address, buf, 2, false);
}
 
static void send_cmd2(const render_context_t* a_ctx, uint8_t a_cmd, uint8_t a_arg)
{
    uint8_t buf[3] = { 0x00, a_cmd, a_arg };
    i2c_write_blocking(i2c0, a_ctx->i2c.address, buf, 3, false);
}

void ssd1306_init(render_context_t* a_ctx, uint16_t a_w, uint16_t a_h, uint16_t a_mhz)
{
    a_ctx->width = a_w;
    a_ctx->height = a_h;
    a_ctx->pixel_size = 1;
    a_ctx->mhz = 0;  // unused for I2C
    (void)a_mhz;
 
    a_ctx->i2c.pin_sda = 4;
    a_ctx->i2c.pin_scl = 5;
    a_ctx->i2c.address = 0x3C;
    a_ctx->i2c.col_start = 0;
    a_ctx->i2c.col_end = 0;
    a_ctx->i2c.row_start = 0;
    a_ctx->i2c.row_end = 0;
    memset(&a_ctx->buffer, 0, sizeof(a_ctx->buffer));

 
    i2c_init(i2c0, 400000);
    gpio_set_function(a_ctx->i2c.pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(a_ctx->i2c.pin_scl, GPIO_FUNC_I2C);
    gpio_pull_up(a_ctx->i2c.pin_sda);
    gpio_pull_up(a_ctx->i2c.pin_scl);
 
    sleep_ms(100);
 
    send_cmd (a_ctx, 0xAE);          // display off
    send_cmd2(a_ctx, 0xD5, 0x80);    // clock divide ratio
    send_cmd2(a_ctx, 0xA8, a_h - 1); // mux ratio
    send_cmd2(a_ctx, 0xD3, 0x00);    // display offset
    send_cmd (a_ctx, 0x40);          // start line = 0
    send_cmd2(a_ctx, 0x8D, 0x14);    // charge pump on
    send_cmd2(a_ctx, 0x20, 0x00);    // horizontal addressing mode
    send_cmd (a_ctx, 0xA1);          // segment remap
    send_cmd (a_ctx, 0xC8);          // COM scan direction
    send_cmd2(a_ctx, 0xDA, 0x12);    // COM pins
    send_cmd2(a_ctx, 0x81, 0xCF);    // contrast
    send_cmd2(a_ctx, 0xD9, 0xF1);    // pre-charge
    send_cmd2(a_ctx, 0xDB, 0x40);    // VCOMH
    send_cmd (a_ctx, 0xA4);          // output follows RAM
    send_cmd (a_ctx, 0xA6);          // normal (non-inverted)
    send_cmd (a_ctx, 0xAF);          // display on
}

void ssd1306_draw_pixel(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_color)
{
    uint8_t* dst = &a_ctx->buffer[1];
    if (a_color)
        dst[(a_y / 8) * a_ctx->width + a_x] |=  (1 << (a_y % 8));
    else
        dst[(a_y / 8) * a_ctx->width + a_x] &= ~(1 << (a_y % 8));
}

void ssd1306_draw_rect(render_context_t* a_ctx, uint16_t a_x, uint16_t a_y, uint16_t a_w, uint16_t a_h, uint16_t a_color)
{
    for (int y = a_y; y < a_y + a_h; y++)
    {
        for (int x = a_x; x < a_x + a_w; x++)
        {
            ssd1306_draw_pixel(a_ctx, x, y, a_color);
        }
    }
}

void ssd1306_draw_glyphs(render_context_t* a_ctx, const char* a_str, uint16_t a_len, uint16_t a_spacing, uint16_t a_scale, uint16_t a_color, uint16_t a_x, uint16_t a_y)
{
    int glyph_w  = 8 * a_scale;
    int glyph_h = 16 * a_scale;
    for (int i = 0; i < a_len; i++)
    {
        const uint8_t* glyph = font8x16[a_str[i] - FONT_FIRST_CHAR];
        int x_offset = i * (glyph_w + a_spacing);
        for (int y = 0; y < glyph_h; y++)
        {
            uint8_t bits = glyph[y / a_scale];
            for (int x = 0; x < glyph_w; x++)
            {
                uint16_t color = (bits & (1 << (7 - x / a_scale))) ? a_color : 0x0000;
                ssd1306_draw_pixel(a_ctx, a_x + x + x_offset, a_y + y, color);
            }
        }
    }
}

void ssd1306_flush(render_context_t* a_ctx)
{
    uint8_t pages = a_ctx->height / 8;
    uint8_t w = a_ctx->width;

    uint8_t col_cmd[4]  = { 0x00, 0x21, SSD1306_COL_OFFSET, SSD1306_COL_OFFSET + w - 1 };
    uint8_t page_cmd[4] = { 0x00, 0x22, 0, pages - 1 };
    i2c_write_blocking(i2c0, a_ctx->i2c.address, col_cmd,  4, false);
    i2c_write_blocking(i2c0, a_ctx->i2c.address, page_cmd, 4, false);

    a_ctx->buffer[0] = 0x40;
    i2c_write_blocking(i2c0, a_ctx->i2c.address, a_ctx->buffer, pages * w + 1, false);
}
