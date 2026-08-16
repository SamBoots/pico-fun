#include "pico/stdlib.h"
#include "app.h"
#include "ammo_counter.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"

typedef struct gun_context_t
{
    button_context_t fire_button;
    button_context_t reload_button;

    uint16_t max_ammo;
    uint16_t current_ammo;
    uint16_t fire_rate_ms;
    uint16_t last_shot_ms;
} gun_context_t;

static bool gun_fire(gun_context_t* a_gun_ctx, uint32_t a_now_ms)
{
    if (a_gun_ctx->current_ammo && (uint32_t)(a_now_ms - a_gun_ctx->last_shot_ms) > a_gun_ctx->fire_rate_ms)
    {
        a_gun_ctx->current_ammo = a_gun_ctx->current_ammo - 1;
        return true;
    }
    return false;
}

static inline bool gun_reload(gun_context_t* a_gun_ctx)
{
    a_gun_ctx->current_ammo = a_gun_ctx->max_ammo;
    return true;
}

static void gun_update_screen(gun_context_t* a_gun_ctx, render_context_t* a_ctx)
{
    char digits[] = {'0', '0'};
    if (a_gun_ctx->current_ammo)
    {
        digits[0] = '0' + (a_gun_ctx->current_ammo / 10);
        digits[1] = '0' + (a_gun_ctx->current_ammo % 10);
    }
    uint16_t scale = 12;
    render_8x16glyphs(a_ctx, digits, 2, 1, scale, COLOR_RED, COLOR_BLACK, 24, 24);
    render_flush(a_ctx);
}

void ammo_counter_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(gun_context_t));
    a_app->update = ammo_counter_update;
    a_app->render = ammo_counter_render;
    a_app->close = ammo_counter_close;
    
    gun_context_t* gun_ctx = (gun_context_t*)a_app->user_data;

    gun_ctx->max_ammo = 10;
    gun_ctx->current_ammo = 42;
    gun_ctx->fire_rate_ms = 80;
    gun_ctx->last_shot_ms = 0;

    button_init_context(&gun_ctx->fire_button, 15, 10);
    button_init_context(&gun_ctx->reload_button, 17, 10);

    render_fill(a_ctx, COLOR_BLACK);
    gun_update_screen(gun_ctx, a_ctx);
}

bool ammo_counter_update(app_context_t* a_app, uint32_t a_now_ms)
{
    gun_context_t* gun_ctx = (gun_context_t*)a_app->user_data;
    button_update(&gun_ctx->fire_button, a_now_ms);
    button_update(&gun_ctx->reload_button, a_now_ms);
    if (button_pressed(&gun_ctx->fire_button))
    {
        if (gun_fire(gun_ctx, a_now_ms))
            return true;
    }

    if (button_pressed(&gun_ctx->reload_button))
    {
        if (gun_reload(gun_ctx))
            return true;
    }
    return false;
}

void ammo_counter_render(app_context_t* a_app, render_context_t* a_ctx)
{
    gun_context_t* gun_ctx = (gun_context_t*)a_app->user_data;
    gun_update_screen(gun_ctx, a_ctx);
}

void ammo_counter_close(app_context_t* a_app)
{
    gun_context_t* gun_ctx = (gun_context_t*)a_app->user_data;
    button_free_context(&gun_ctx->fire_button);
    button_free_context(&gun_ctx->reload_button);
}
