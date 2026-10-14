#include "defines/types.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "app.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"
#include "io/io_types.h"
#include "io/button.h"
#include "memory/memory_arena.h"

#define MAX_SNAKE 128 // cheat

static inline isize wrap(isize a_val, isize a_max) 
{
    return ((a_val % a_max) + a_max) % a_max;
}

typedef struct snake_params_t
{
    u8 scale;
} snake_params_t;

static i16 dirs[2][4] =
{
    { 1, 0, -1, 0 },
    { 0, 1, 0, -1 }
};

typedef struct snake_context_t
{
    button_context_t left_button;
    button_context_t right_button;

    u32 ms_last_frame;
    u32 ms_per_frame;
    
    u16 map_scale;
    u16 map_x;
    u16 map_y;
    u8* map;

    u16 cur_dir;

    u16 snake[MAX_SNAKE];
    u16 snake_size;
} snake_context_t;

static void snake_set_tile(snake_context_t* a_snake_ctx, u32 a_index, bool a_active)
{
    u32 mask = 1 << (a_index & 7);
    u32 byte = a_index >> 3;
    a_snake_ctx->map[byte] = (a_snake_ctx->map[byte] & ~mask) | (a_active << (a_index & 7));
}

static inline bool snake_read_tile(snake_context_t* a_snake_ctx, u32 a_index)
{
    return (a_snake_ctx->map[a_index >> 3] >> (a_index & 7)) & 1;
}

static void snake_place_apple(snake_context_t* a_snake_ctx)
{
    u16 apple_pos = rand() % (a_snake_ctx->map_x * a_snake_ctx->map_y);
    for (isize i = 0; i < a_snake_ctx->snake_size; i++)
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
    i16 x = a_snake_ctx->snake[0] % a_snake_ctx->map_x + dirs[0][a_snake_ctx->cur_dir];
    i16 y = a_snake_ctx->snake[0] / a_snake_ctx->map_x + dirs[1][a_snake_ctx->cur_dir];
    if (x > a_snake_ctx->map_x || x < 0 || y > a_snake_ctx->map_y || y < 0)
        return false;

    u16 new_head = y * a_snake_ctx->map_x + x;
    u16 old_tail = a_snake_ctx->snake[a_snake_ctx->snake_size - 1];

    // update snek
    for (isize i = a_snake_ctx->snake_size - 1; i > 0; i--)
        a_snake_ctx->snake[i] = a_snake_ctx->snake[i - 1];

    for (isize i = 0; i < a_snake_ctx->snake_size - 1; i++)
        if (a_snake_ctx->snake[i] == new_head)
            return false;
    
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

    a_snake_ctx->cur_dir = 0;
    a_snake_ctx->snake[0] = a_snake_ctx->map_y / 2 * a_snake_ctx->map_x + a_snake_ctx->map_x / 4;
    a_snake_ctx->snake[1] = a_snake_ctx->snake[0] - 1;
    a_snake_ctx->snake_size = 2;
    snake_set_tile(a_snake_ctx, a_snake_ctx->snake[0], true);
    snake_set_tile(a_snake_ctx, a_snake_ctx->snake[1], true);
    snake_place_apple(a_snake_ctx);
}

app_update_status_t snake_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, u32 a_now_ms)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    if ((u32)(a_now_ms - snake_ctx->ms_last_frame) > snake_ctx->ms_per_frame)
    {
        button_update(&snake_ctx->left_button, a_now_ms);
        button_update(&snake_ctx->right_button, a_now_ms);

        if (button_pressed(&snake_ctx->left_button))
        {
            snake_ctx->cur_dir = wrap(snake_ctx->cur_dir + 1, 4);
        }
        if (button_pressed(&snake_ctx->right_button))
        {
            snake_ctx->cur_dir = wrap(snake_ctx->cur_dir - 1, 4);
        }

        snake_ctx->ms_last_frame = a_now_ms;

        if (!snake_move(snake_ctx))
            snake_start_game(snake_ctx);
        return APP_RENDER;
    }
    return APP_OK;
}

void snake_render(app_context_t* a_app, render_context_t* a_ctx)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    render_draw_rect(a_ctx, 0, snake_ctx->map_x * snake_ctx->map_scale, 0, snake_ctx->map_y * snake_ctx->map_scale, 0);
    for (isize x = 0; x < snake_ctx->map_x * snake_ctx->map_scale; x += snake_ctx->map_scale)
        for (isize y = 0; y < snake_ctx->map_y * snake_ctx->map_scale; y += snake_ctx->map_scale)
            if (snake_read_tile(snake_ctx, (y / snake_ctx->map_scale) * snake_ctx->map_x + (x / snake_ctx->map_scale)))
                render_draw_rect(a_ctx, x, x + snake_ctx->map_scale, y, y + snake_ctx->map_scale, 255);
    render_flush(a_ctx);
}

void snake_close(app_context_t* a_app)
{
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    button_free_context(&snake_ctx->left_button);
    button_free_context(&snake_ctx->right_button);
}

void snake_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count)
{
    *a_param_buf_size = sizeof(snake_params_t);
    *a_desc_count = 1;
}

void snake_default_params(u8* a_param_buf, app_param_descriptor_t* a_descs)
{
    snake_params_t* defaults = (snake_params_t*)a_param_buf;
    defaults->scale = 16;

    a_descs[0] = APP_PARAM("scale", PARAM_U8, offsetof(snake_params_t, scale), 8, 32);
}

void snake_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params)
{
    const snake_params_t* params = (const snake_params_t*)a_app_params;

    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(snake_context_t));
    a_app->update = snake_update;
    a_app->render = snake_render;
    a_app->close = snake_close;
    
    snake_context_t* snake_ctx = (snake_context_t*)a_app->user_data;
    snake_ctx->ms_last_frame = 0;
    snake_ctx->ms_per_frame = 250;

    snake_ctx->map_scale = params->scale;
    snake_ctx->map_x = a_ctx->width / snake_ctx->map_scale;
    snake_ctx->map_y = a_ctx->height / snake_ctx->map_scale;
    snake_ctx->map = memory_arena_allocate(a_arena, (snake_ctx->map_x * snake_ctx->map_y + 7) / 8);

    button_init_context(&snake_ctx->left_button, 2, 10);
    button_init_context(&snake_ctx->right_button, 3, 10);

    snake_start_game(snake_ctx);
}

APP_REGISTER(snake);
