#include <stdlib.h>
#include "pico/stdlib.h"
#include "app.h"
#include "tetris.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"

typedef struct tetris_context_t
{
    button_context_t north_button;
    button_context_t south_button;
    button_context_t west_button;
    button_context_t east_button;

    uint32_t ms_last_frame;
    uint32_t ms_per_frame;

    uint16_t map_scale;
    uint16_t map_x;
    uint16_t map_y;
    uint8_t* map;
    
    // tbd: piece type & location
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

static void tetris_start_game(tetris_context_t* a_tetris_ctx)
{
    memory_set(a_tetris_ctx->map, 0, (a_tetris_ctx->map_x * a_tetris_ctx->map_y + 7) / 8);
    // tbd: spawn new piece
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
    tetris_ctx->map_x = a_ctx->width / tetris_ctx->map_scale;
    tetris_ctx->map_y = a_ctx->height / tetris_ctx->map_scale;
    tetris_ctx->map = memory_arena_allocate(a_arena, (tetris_ctx->map_x * tetris_ctx->map_y + 7) / 8);

    button_init_context(&tetris_ctx->north_button, 2, 10);
    button_init_context(&tetris_ctx->south_button, 3, 10);
    button_init_context(&tetris_ctx->west_button, 9, 10);
    button_init_context(&tetris_ctx->east_button, 13, 10);

    tetris_start_game(tetris_ctx);
}

bool tetris_update(app_context_t* a_app, uint32_t a_now_ms)
{
    tetris_context_t* tetris_ctx = (tetris_context_t*)a_app->user_data;
    if ((uint32_t)(a_now_ms - tetris_ctx->ms_last_frame) > tetris_ctx->ms_per_frame)
    {
        button_update(&tetris_ctx->north_button, a_now_ms);
        button_update(&tetris_ctx->south_button, a_now_ms);
        button_update(&tetris_ctx->west_button, a_now_ms);
        button_update(&tetris_ctx->east_button, a_now_ms);

        if (button_pressed(&tetris_ctx->west_button))
        {
            // tbd: piece move left
        }
        else if (button_pressed(&tetris_ctx->east_button))
        {
            // tbd: piece move right
        }
        else if (button_pressed(&tetris_ctx->north_button))
        {
            // tbd: rotate piece
        }

        // tbd: piece fall & fast drop button
    }
    return true;
}

void tetris_render(app_context_t* a_app, render_context_t* a_ctx)
{
    render_draw_rect(a_ctx, 0, 0, a_ctx->width, a_ctx->height, 0);
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
    render_flush(a_ctx);
}

void tetris_close(app_context_t* a_app)
{
    tetris_context_t* tetris_ctx = (tetris_context_t*)a_app->user_data;
    button_free_context(&tetris_ctx->north_button);
    button_free_context(&tetris_ctx->south_button);
    button_free_context(&tetris_ctx->west_button);
    button_free_context(&tetris_ctx->east_button);
}