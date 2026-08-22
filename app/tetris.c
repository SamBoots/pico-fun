#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "app.h"
#include "tetris.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"

static const uint16_t pieces[7][4] =
{
    {0x0F00, 0X2222, 0X0F00, 0X2222}, // I
    {0X0660, 0X0660, 0X0660, 0X0660}, // O
    {0X0E40, 0X4C40, 0X4E00, 0X4640}, // T
    {0X4460, 0X0E80, 0XC440, 0X2E00}, // J
    {0X44C0, 0X8E00, 0X6440, 0X0E20}, // L
    {0X06C0, 0X4C40, 0X4E00, 0X4640}, // S
    {0X0C60, 0X2640, 0X0C60, 0X2640}  // Z
};

typedef struct tetris_context_t
{
    button_context_t left_button;
    button_context_t right_button;
    button_context_t rotate_button;
    button_context_t suiside_button;

    uint32_t ms_last_frame;
    uint32_t ms_per_frame;

    uint16_t map_scale;
    uint16_t map_x;
    uint16_t map_y;
    uint8_t* map;

    uint16_t playfield_origin_x;
    uint16_t playfield_origin_y;
    uint16_t info_panel_x;
    uint16_t font_scale;
    
    uint8_t current_piece;
    uint8_t current_pose;
    uint8_t current_loc_x;
    uint8_t current_loc_y;
    uint8_t next_piece;

    uint16_t score;
    uint16_t highscore;
} tetris_context_t;

static void tetris_set_tile(tetris_context_t* a_tetris_ctx, uint32_t a_index, bool a_active)
{
    uint32_t mask = 1 << (a_index & 7);
    uint32_t byte = a_index >> 3;
    a_tetris_ctx->map[byte] = (a_tetris_ctx->map[byte] & ~mask) | (a_active << (a_index & 7));
}

static inline bool tetris_read_tile(tetris_context_t* a_tetris_ctx, uint32_t a_index)
{
    return (a_tetris_ctx->map[a_index >> 3] >> (a_index & 7)) & 1;
}

static void tetris_spawn_piece(tetris_context_t* a_tetris_ctx)
{
    a_tetris_ctx->current_piece = rand() % 7;
    a_tetris_ctx->current_pose = 0;
    a_tetris_ctx->current_loc_x = a_tetris_ctx->map_x / 2 - 2;
    a_tetris_ctx->current_loc_y = 0;
}

static bool tetris_can_move(tetris_context_t* a_tetris_ctx, uint16_t new_x, uint16_t new_y, uint16_t new_pose)
{
    uint16_t piece = pieces[a_tetris_ctx->current_piece][new_pose];
    for (int i = 0; i < 16; i++)
    {
        if ((piece >> (15 - i)) & 1)
        {
            int pixel_x = (int)new_x + (i % 4);
            int pixel_y = (int)new_y + (i / 4);
            if (pixel_x < 0 || pixel_x >= a_tetris_ctx->map_x || pixel_y >= a_tetris_ctx->map_y)
            {
                return false;
            }
            
            uint32_t index = pixel_y * a_tetris_ctx->map_x + pixel_x;
            if (tetris_read_tile(a_tetris_ctx, index))
            {
                return false;
            }
        }
    }
    return true;
}

static void tetris_start_game(tetris_context_t* a_tetris_ctx)
{
    a_tetris_ctx->score = 0;

    memory_set(a_tetris_ctx->map, 0, (a_tetris_ctx->map_x * a_tetris_ctx->map_y + 7) / 8);
    tetris_spawn_piece(a_tetris_ctx);
}

static void tetris_check_full_lines(tetris_context_t* a_tetris_ctx)
{
    static const uint16_t line_score[5] = {0, 40, 100, 300, 1200};
    uint8_t lines_cleared = 0;

    for (int y = a_tetris_ctx->map_y - 1; y >= 0; y--)
    {
        bool full_line = true;
        for (int x = 0; x < a_tetris_ctx->map_x; x++)
        {
            if (!tetris_read_tile(a_tetris_ctx, y * a_tetris_ctx->map_x + x))
            {
                full_line = false;
                break;
            }
        }

        if (full_line)
        {
            for (int move_y = y; move_y > 0; move_y--)
            {
                for (int x = 0; x < a_tetris_ctx->map_x; x++)
                {
                    uint32_t old_index = (move_y - 1) * a_tetris_ctx->map_x + x;
                    uint32_t new_index = old_index + a_tetris_ctx->map_x;
                    bool old_tile = tetris_read_tile(a_tetris_ctx, old_index);
                    tetris_set_tile(a_tetris_ctx, new_index, old_tile);
                }
            }

            for (int x = 0; x < a_tetris_ctx->map_x; x++)
            {
                tetris_set_tile(a_tetris_ctx, x, false);
            }
            y++;
        }
    }

    if (lines_cleared > 0)
    {
        a_tetris_ctx->score += line_score[lines_cleared];
    }
}

static void tetris_piece_die(tetris_context_t* a_tetris_ctx)
{
    uint16_t piece = pieces[a_tetris_ctx->current_piece][a_tetris_ctx->current_pose];
    for (int i = 0; i < 16; i++)
    {
        if ((piece >> (15 - i)) & 1)
        {
            uint16_t pixel_x = a_tetris_ctx->current_loc_x + (i % 4);
            uint16_t pixel_y = a_tetris_ctx->current_loc_y + (i / 4);
            uint32_t index = pixel_y * a_tetris_ctx->map_x + pixel_x;
            tetris_set_tile(a_tetris_ctx, index, true);
        }
    }
    tetris_check_full_lines(a_tetris_ctx);
    tetris_spawn_piece(a_tetris_ctx);

    if (!tetris_can_move(a_tetris_ctx, a_tetris_ctx->current_loc_x, a_tetris_ctx->current_loc_y, a_tetris_ctx->current_pose))
    {
        tetris_start_game(a_tetris_ctx);
    }
}

void tetris_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(tetris_context_t));
    a_app->update = tetris_update;
    a_app->render = tetris_render;
    a_app->close = tetris_close;
    
    tetris_context_t* tetris_ctx = (tetris_context_t*)a_app->user_data;
    tetris_ctx->ms_last_frame = 0;
    tetris_ctx->ms_per_frame = 250;

    tetris_ctx->map_scale = 8;
    const uint16_t info_panel_width = 48;
    tetris_ctx->playfield_origin_x = 1;
    tetris_ctx->playfield_origin_y = 1;
    uint16_t playfield_width = a_ctx->width - info_panel_width - 2;
    uint16_t playfield_height = a_ctx->height - 2;
    tetris_ctx->map_x = playfield_width / tetris_ctx->map_scale;
    tetris_ctx->map_y = playfield_height / tetris_ctx->map_scale;
    tetris_ctx->map = memory_arena_allocate(a_arena, (tetris_ctx->map_x * tetris_ctx->map_y + 7) / 8);

    tetris_ctx->info_panel_x = tetris_ctx->playfield_origin_x + tetris_ctx->map_x * tetris_ctx->map_scale + 5;
    tetris_ctx->font_scale = 2;
    tetris_ctx->score = 0;
    tetris_ctx->highscore = 0;
    tetris_ctx->next_piece = rand() % 7;

    button_init_context(&tetris_ctx->rotate_button, 15, 10);
    button_init_context(&tetris_ctx->suiside_button, 17, 10);
    button_init_context(&tetris_ctx->left_button, 16, 10);
    button_init_context(&tetris_ctx->right_button, 20, 10);

    render_fill(a_ctx, COLOR_BLACK);
    render_flush(a_ctx);

    tetris_start_game(tetris_ctx);
}

bool tetris_update(app_context_t* a_app, uint32_t a_now_ms)
{
    tetris_context_t* tetris_ctx = (tetris_context_t*)a_app->user_data;
    if ((uint32_t)(a_now_ms - tetris_ctx->ms_last_frame) > tetris_ctx->ms_per_frame)
    {
        tetris_ctx->ms_last_frame = a_now_ms;

        button_update(&tetris_ctx->left_button, a_now_ms);
        button_update(&tetris_ctx->right_button, a_now_ms);
        button_update(&tetris_ctx->rotate_button, a_now_ms);
        button_update(&tetris_ctx->suiside_button, a_now_ms);

        if (button_pressed(&tetris_ctx->left_button))
        {
            if (tetris_can_move(tetris_ctx, tetris_ctx->current_loc_x - 1, tetris_ctx->current_loc_y, tetris_ctx->current_pose))
            {
                tetris_ctx->current_loc_x--;
            }
        }
        else if (button_pressed(&tetris_ctx->right_button))
        {
            if (tetris_can_move(tetris_ctx, tetris_ctx->current_loc_x + 1, tetris_ctx->current_loc_y, tetris_ctx->current_pose))
            {
                tetris_ctx->current_loc_x++;
            }
        }
        else if (button_pressed(&tetris_ctx->rotate_button))
        {
            uint8_t new_pose = (tetris_ctx->current_pose + 1) % 4;
            if (tetris_can_move(tetris_ctx, tetris_ctx->current_loc_x, tetris_ctx->current_loc_y, new_pose))
            {
                tetris_ctx->current_pose = new_pose;
            }
        }
        else if (button_pressed(&tetris_ctx->suiside_button))
        {
            while (tetris_can_move(tetris_ctx, tetris_ctx->current_loc_x, tetris_ctx->current_loc_y + 1, tetris_ctx->current_pose))
            {
                tetris_ctx->current_loc_y++;
            }
            tetris_piece_die(tetris_ctx);
        }

        if (tetris_can_move(tetris_ctx, tetris_ctx->current_loc_x, tetris_ctx->current_loc_y + 1, tetris_ctx->current_pose))
        {
            tetris_ctx->current_loc_y++;
        }
        else
        {
            tetris_piece_die(tetris_ctx);
        }
        return true;
    }
    return false;
}

static void render_border(tetris_context_t* a_tetris_ctx, render_context_t* a_ctx)
{
    uint16_t x0 = a_tetris_ctx->playfield_origin_x - 1;
    uint16_t y0 = a_tetris_ctx->playfield_origin_y - 1;
    uint16_t x1 = a_tetris_ctx->playfield_origin_x + a_tetris_ctx->map_x * a_tetris_ctx->map_scale;
    uint16_t y1 = a_tetris_ctx->playfield_origin_y + a_tetris_ctx->map_y * a_tetris_ctx->map_scale;

    for (uint16_t x = x0; x <= x1; x++)
    {
        render_draw_pixel(a_ctx, x, y0, COLOR_WHITE);
        render_draw_pixel(a_ctx, x, y1, COLOR_WHITE);
    }
    for (uint16_t y = y0; y <= y1; y++)
    {
        render_draw_pixel(a_ctx, x0, y, COLOR_WHITE);
        render_draw_pixel(a_ctx, x1, y, COLOR_WHITE);
    }
}

static void render_forecast(tetris_context_t* a_tetris_ctx, render_context_t* a_ctx)
{
    uint16_t area_x = a_tetris_ctx->info_panel_x + 4;
    uint16_t area_y = 4;
    uint16_t grid_y = area_y + 20;

    render_4x8glyphs(a_ctx, "NEXT", 4, 1, a_tetris_ctx->font_scale, COLOR_WHITE, COLOR_BLACK, area_x, area_y);
    render_draw_rect(a_ctx, area_x, grid_y, 32, 32, COLOR_BLACK);

    uint16_t piece = pieces[a_tetris_ctx->next_piece][0];
    for (int i = 0; i < 16; i++)
    {
        if ((piece >> (15 - i)) & 1)
        {
            uint16_t tile_x = (i % 4);
            uint16_t tile_y = (i / 4);
            for (int x = 0; x < a_tetris_ctx->map_scale; x++)
            {
                for (int y = 0; y < a_tetris_ctx->map_scale; y++)
                {
                    uint16_t pixel_x = area_x + (tile_x * a_tetris_ctx->map_scale) + x;
                    uint16_t pixel_y = grid_y + (tile_y * a_tetris_ctx->map_scale) + y;
                    render_draw_pixel(a_ctx, pixel_x, pixel_y, COLOR_WHITE);
                }
            }
        }
    }
}

static void render_score(tetris_context_t* a_tetris_ctx, render_context_t* a_ctx)
{
    uint16_t x = a_tetris_ctx->info_panel_x;
    uint16_t y = 64;
    uint16_t line_height = 18;

    char buf[6];
    render_4x8glyphs(a_ctx, "SCORE", 5, 1, a_tetris_ctx->font_scale, COLOR_WHITE, COLOR_BLACK, x, y);
    snprintf(buf, sizeof(buf), "%05d", a_tetris_ctx->score);
    render_draw_rect(a_ctx, x, y + line_height, 48, 16, COLOR_BLACK);
    render_4x8glyphs(a_ctx, buf, 5, 1, a_tetris_ctx->font_scale, COLOR_WHITE, COLOR_BLACK, x, y + line_height);
    
    y += line_height * 2 + 6;
    render_4x8glyphs(a_ctx, "HIGH", 4, 1, a_tetris_ctx->font_scale, COLOR_WHITE, COLOR_BLACK, x, y);
    snprintf(buf, sizeof(buf), "%05d", a_tetris_ctx->highscore);
    render_draw_rect(a_ctx, x, y + line_height, 48, 16, COLOR_BLACK);
    render_4x8glyphs(a_ctx, buf, 5, 1, a_tetris_ctx->font_scale, COLOR_WHITE, COLOR_BLACK, x, y + line_height);
}

void tetris_render(app_context_t* a_app, render_context_t* a_ctx)
{
    tetris_context_t* tetris_ctx = (tetris_context_t*)a_app->user_data;
    for (int x = 0; x < tetris_ctx->map_x * tetris_ctx->map_scale; x++)
    {
        for (int y = 0; y < tetris_ctx->map_y * tetris_ctx->map_scale; y++)
        {
            if (tetris_read_tile(tetris_ctx, (y / tetris_ctx->map_scale) * tetris_ctx->map_x + (x / tetris_ctx->map_scale)))
            {
                render_draw_pixel(a_ctx, x, y, 255);
            }
            else
            {
                render_draw_pixel(a_ctx, x, y, 0);
            }  
        }
    }

    uint16_t piece = pieces[tetris_ctx->current_piece][tetris_ctx->current_pose];
    for (int i = 0; i < 16; i++)
    {
        if ((piece >> (15 - i)) & 1)
        {
            uint16_t tile_x = tetris_ctx->current_loc_x + (i % 4);
            uint16_t tile_y = tetris_ctx->current_loc_y + (i / 4);
            for (int x = 0; x < tetris_ctx->map_scale; x++)
            {
                for (int y = 0; y < tetris_ctx->map_scale; y++)
                {
                    uint16_t pixel_x = tetris_ctx->playfield_origin_x + (tile_x * tetris_ctx->map_scale) + x;
                    uint16_t pixel_y = tetris_ctx->playfield_origin_y + (tile_y * tetris_ctx->map_scale) + y;
                    render_draw_pixel(a_ctx, pixel_x, pixel_y, COLOR_WHITE);
                }
            }
        }
    }

    render_border(tetris_ctx, a_ctx);
    render_forecast(tetris_ctx, a_ctx);
    render_score(tetris_ctx, a_ctx);

    render_flush(a_ctx);
}

void tetris_close(app_context_t* a_app)
{
    tetris_context_t* tetris_ctx = (tetris_context_t*)a_app->user_data;
    button_free_context(&tetris_ctx->left_button);
    button_free_context(&tetris_ctx->right_button);
    button_free_context(&tetris_ctx->rotate_button);
    button_free_context(&tetris_ctx->suiside_button);
}