#include <stdlib.h>
#include "pico/stdlib.h"
#include "app.h"
#include "snake.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"

#define MAX_SNAKE 128 // cheat

typedef struct snake_context_t
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

    uint16_t snake[MAX_SNAKE];
    int16_t snake_dir_x;
    int16_t snake_dir_y;
    uint16_t snake_size;
} snake_context_t;

static void snake_set_tile(snake_context_t* a_snake_ctx, uint32_t a_index, bool a_active)
{
    uint32_t mask = 1 << (a_index & 7);
    uint32_t byte = a_index >> 3;
    a_snake_ctx->map[byte] = (a_snake_ctx->map[byte] & ~mask) | (a_active << (a_index & 7));
}

static inline bool snake_read_tile(snake_context_t* a_snake_ctx, uint32_t a_index)
{
    return (a_snake_ctx->map[a_index >> 3] >> (a_index & 7)) & 1;
}

static void snake_place_apple(snake_context_t* a_snake_ctx)
{
    uint16_t apple_pos = rand() % (a_snake_ctx->map_x * a_snake_ctx->map_y);
    for (int i = 0; i < a_snake_ctx->snake_size; i++)
    {
        if (a_snake_ctx->snake[i] == apple_pos)
        {
            snake_place_apple(a_snake_ctx);
            return;
        }
    }
    snake_set_tile(a_snake_ctx, apple_pos, true);
}

static bool snake_move(snake_context_t* a_snake_ctx)
{
    int16_t x = a_snake_ctx->snake[0] % a_snake_ctx->map_x + a_snake_ctx->snake_dir_x;
    int16_t y = a_snake_ctx->snake[0] / a_snake_ctx->map_x + a_snake_ctx->snake_dir_y;
    if (x >= a_snake_ctx->map_x || x < 0 || y >= a_snake_ctx->map_y || y < 0)
    {
        return false;
    }

    uint16_t new_head = y * a_snake_ctx->map_x + x;
    uint16_t old_tail = a_snake_ctx->snake[a_snake_ctx->snake_size - 1];

    // update snek
    for (int i = a_snake_ctx->snake_size - 1; i > 0; i--)
        a_snake_ctx->snake[i] = a_snake_ctx->snake[i - 1];

    for (int i = 0; i < a_snake_ctx->snake_size - 1; i++)
    {
        if (a_snake_ctx->snake[i] == new_head)
            return false;
    }
    
    if (snake_read_tile(a_snake_ctx, new_head))
    {
        snake_place_apple(a_snake_ctx);
        a_snake_ctx->snake[a_snake_ctx->snake_size] = old_tail; 
        ++a_snake_ctx->snake_size;
    }
    else
        snake_set_tile(a_snake_ctx, old_tail, false);
    a_snake_ctx->snake[0] = new_head;
    snake_set_tile(a_snake_ctx, new_head, true);

    return true;
}

static void snake_start_game(snake_context_t* a_snake_ctx)
{
    memory_set(a_snake_ctx->map, 0, (a_snake_ctx->map_x * a_snake_ctx->map_y + 7) / 8);

    a_snake_ctx->snake_dir_x = 1;
    a_snake_ctx->snake_dir_y = 0;
    a_snake_ctx->snake[0] = a_snake_ctx->map_y / 2 * a_snake_ctx->map_x + a_snake_ctx->map_x / 4;
    a_snake_ctx->snake[1] = a_snake_ctx->snake[0] - 1;
    a_snake_ctx->snake_size = 2;
    snake_set_tile(a_snake_ctx, a_snake_ctx->snake[0], true);
    snake_set_tile(a_snake_ctx, a_snake_ctx->snake[1], true);
    snake_place_apple(a_snake_ctx);
}

void snake_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(snake_context_t));
    a_app->update = snake_update;
    a_app->render = snake_render;
    a_app->close = snake_close;
    
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    snake_ctx->ms_last_frame = 0;
    snake_ctx->ms_per_frame = 250;

    snake_ctx->map_scale = 8;
    snake_ctx->map_x = a_ctx->width / snake_ctx->map_scale;
    snake_ctx->map_y = a_ctx->height / snake_ctx->map_scale;
    snake_ctx->map = memory_arena_allocate(a_arena, (snake_ctx->map_x * snake_ctx->map_y + 7) / 8);

    button_init_context(&snake_ctx->north_button, 2, 10);
    button_init_context(&snake_ctx->south_button, 3, 10);
    button_init_context(&snake_ctx->west_button, 9, 10);
    button_init_context(&snake_ctx->east_button, 13, 10);

    snake_start_game(snake_ctx);
}

bool snake_update(app_context_t* a_app, uint32_t a_now_ms)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    if ((uint32_t)(a_now_ms - snake_ctx->ms_last_frame) > snake_ctx->ms_per_frame)
    {
        button_update(&snake_ctx->north_button, a_now_ms);
        button_update(&snake_ctx->south_button, a_now_ms);
        button_update(&snake_ctx->west_button, a_now_ms);
        button_update(&snake_ctx->east_button, a_now_ms);

        if (button_pressed(&snake_ctx->west_button))
        {
            snake_ctx->snake_dir_x = 1;
            snake_ctx->snake_dir_y = 0;
        }
        else if (button_pressed(&snake_ctx->east_button))
        {
            snake_ctx->snake_dir_x = -1;
            snake_ctx->snake_dir_y = 0;
        }
        else if (button_pressed(&snake_ctx->south_button))
        {
            snake_ctx->snake_dir_x = 0;
            snake_ctx->snake_dir_y = 1;
        }
        else if (button_pressed(&snake_ctx->north_button))
        {
            snake_ctx->snake_dir_x = 0;
            snake_ctx->snake_dir_y = -1;
        }

        snake_ctx->ms_last_frame = a_now_ms;

        if (!snake_move(snake_ctx))
            snake_start_game(snake_ctx);
    }
    return true;
}

void snake_render(app_context_t* a_app, render_context_t* a_ctx)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    for (int x = 0; x < snake_ctx->map_x * snake_ctx->map_scale; x++)
    {
        for (int y = 0; y < snake_ctx->map_y * snake_ctx->map_scale; y++)
        {
            if (snake_read_tile(snake_ctx, (y / snake_ctx->map_scale) * snake_ctx->map_x + (x / snake_ctx->map_scale)))
                render_draw_pixel(a_ctx, x, y, 255);
        }
    }
    render_flush(a_ctx);
}

void snake_close(app_context_t* a_app)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    button_free_context(&snake_ctx->north_button);
    button_free_context(&snake_ctx->south_button);
    button_free_context(&snake_ctx->west_button);
    button_free_context(&snake_ctx->east_button);
}
