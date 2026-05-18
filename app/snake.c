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

    uint16_t map_x;
    uint16_t map_y;
    uint8_t* map;

    uint16_t snake[MAX_SNAKE];
    uint16_t snake_size;
    uint16_t apple_index_pos;
} snake_context_t;

static void snake_set_tile(snake_context_t* a_snake_ctx, uint32_t a_index, bool a_active)
{
    if (a_active)
    {
        a_snake_ctx->map[a_index >> 3] |= (a_index << (a_index & 7));
    }
    else
    {
        a_snake_ctx->map[a_index >> 3] &= ~(a_index << (a_index & 7));
    }
}

static inline bool snake_read_tile(snake_context_t* a_snake_ctx, uint32_t a_index)
{
    return (a_snake_ctx->map[a_index >> 3] >> (a_index & 7)) & 1;
}

static void snake_place_apple(snake_context_t* a_snake_ctx)
{
    snake_set_tile(a_snake_ctx, a_snake_ctx->apple_index_pos, false);
    uint16_t apple_pos = rand() % (a_snake_ctx->map_x * a_snake_ctx->map_y + 1);
    for (int i = 0; i < a_snake_ctx->snake_size; i++)
    {
        if (a_snake_ctx->snake[i] == apple_pos)
        {
            snake_place_apple(a_snake_ctx);
            return;
        }
    }
    a_snake_ctx->apple_index_pos = apple_pos;
    snake_set_tile(a_snake_ctx, a_snake_ctx->apple_index_pos, true);
}

static inline bool snake_move(snake_context_t* a_snake_ctx, int a_x, int a_y)
{
    if (a_x > a_snake_ctx->map_x || a_x < 0 || a_y > a_snake_ctx->map_y || a_y < 0)
    {
        return false;
    }

    uint16_t new_head = a_y * a_snake_ctx->map_x + a_x;
    uint16_t old_tail = a_snake_ctx->snake[a_snake_ctx->snake_size - 1];
    for (int i = 0; i < a_snake_ctx->snake_size; i++)
    {
        if (a_snake_ctx->snake[i] == new_head)
            return false;
    }

    snake_set_tile(a_snake_ctx, new_head, true);
    if (new_head == a_snake_ctx->apple_index_pos)
        snake_place_apple(a_snake_ctx);
    else
        snake_set_tile(a_snake_ctx, old_tail, false);

    // update snek
    for (int i = a_snake_ctx->snake_size; i > 1; i++)
        a_snake_ctx->snake[i] == a_snake_ctx->snake[i - 1];
    a_snake_ctx->snake[0] = new_head;

    return true;
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
    snake_ctx->ms_per_frame = 1000;

    snake_ctx->map_x = a_ctx->width;
    snake_ctx->map_y = a_ctx->height;
    snake_ctx->map = memory_arena_allocate(a_arena, snake_ctx->map_x * snake_ctx->map_y / 8);

    snake_place_apple(snake_ctx);
    snake_ctx->snake[0] = snake_ctx->map_y / 2 * snake_ctx->map_x + snake_ctx->map_x / 2;
    snake_ctx->snake_size = 1;

    button_init_context(&snake_ctx->north_button, 2, 10);
    button_init_context(&snake_ctx->south_button, 3, 10);
    button_init_context(&snake_ctx->west_button, 9, 10);
    button_init_context(&snake_ctx->east_button, 13, 10);
}

bool snake_update(app_context_t* a_app, uint32_t a_now_ms)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    if ((uint32_t)(a_now_ms - snake_ctx->ms_last_frame) > snake_ctx->ms_per_frame)
    {
        snake_ctx->ms_last_frame = a_now_ms;

    }
}

void snake_render(app_context_t* a_app, render_context_t* a_ctx)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    for (int x = 0; x < snake_ctx->map_x; x++)
    {
        for (int y = 0; y < snake_ctx->map_y; y++)
        {
            if (snake_read_tile(snake_ctx, y * snake_ctx->map_x + x))
                render_draw_pixel(a_ctx, x, y, 255);
        }
    }
}

void snake_close(app_context_t* a_app)
{
    (void)a_app;
}
