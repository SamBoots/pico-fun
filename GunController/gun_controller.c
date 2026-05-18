#define TEST_W 64
#define TEST_H 48
 
#include "pico/stdlib.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"
#include "io/io_types.h"
#include "io/button.h"
#include "string.h"

static render_context_t g_render_ctx;

typedef struct gun_context_t
{
    uint16_t max_ammo;
    uint16_t current_ammo;
    uint16_t fire_rate_ms;
    uint16_t last_shot_ms;
} gun_context_t;

static bool gun_fire(gun_context_t* a_ctx, uint32_t a_now_ms)
{
    if (a_ctx->current_ammo && (uint32_t)(a_now_ms - a_ctx->last_shot_ms) > a_ctx->fire_rate_ms)
    {
        a_ctx->current_ammo = a_ctx->current_ammo - 1;
        return true;
    }
    return false;
}

static bool gun_reload(gun_context_t* a_ctx)
{
    a_ctx->current_ammo = a_ctx->max_ammo;
    return true;
}

static void gun_update_screen(gun_context_t* a_ctx)
{
    char digit_one = '0';
    char digit_two = '0';
    if (a_ctx->current_ammo)
    {
        digit_one = '0' + (a_ctx->current_ammo / 10);
        digit_two = '0' + (a_ctx->current_ammo % 10);
    }
    render_8x16numbers(&g_render_ctx, digit_one, 3, 255, 0, 0);
    render_8x16numbers(&g_render_ctx, digit_two, 3, 255, 32, 0);   
    render_flush(&g_render_ctx);
}
 
int main(void)
{
    stdio_init_all();
    
    render_init_context(&g_render_ctx, TEST_W, TEST_H, 125);
    button_context_t fire_button;
    button_context_t reload_button;
    button_init_context(&fire_button, 2, 10);
    button_init_context(&reload_button, 3, 10);

    gun_context_t gun;
    gun.max_ammo = 42;
    gun.current_ammo = 42;
    gun.fire_rate_ms = 80;
    gun.last_shot_ms = 0;
    gun_update_screen(&gun);

    while (true) 
    {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        button_update(&fire_button, now_ms);
        button_update(&reload_button, now_ms);
        if (button_pressed(&fire_button))
        {
            if (gun_fire(&gun, now_ms))
                gun_update_screen(&gun);
        }

        if (button_pressed(&reload_button))
        {
            if (gun_reload(&gun))
                gun_update_screen(&gun);
        }
    }
}
