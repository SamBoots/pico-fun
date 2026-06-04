#define TEST_W 240
#define TEST_H 280
 
#include "pico/stdlib.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"

#include "io/io_types.h"
#include "io/button.h"

#include "memory/memory_arena.h"

#include "app/app.h"
#include "app/ammo_counter.h"
#include "app/snake.h"
#include "app/tetris.h"

#include "stdio.h"

static render_context_t g_render_ctx;
static memory_arena_t g_memory_arena;

typedef struct delay_bool_t
{
    bool value;
    uint32_t last_change_ms;
    uint32_t ms_switch;
} delay_bool_t;

delay_bool_t create_delay_bool_value(uint32_t a_switch_ms, bool a_default)
{
    delay_bool_t val;
    val.value = a_default;
    val.last_change_ms = 0;
    val.ms_switch = a_switch_ms;
    return val;
}

bool get_delay_bool_value(delay_bool_t* a_bool, uint32_t a_now_ms)
{
    if ((uint32_t)(a_now_ms - a_bool->last_change_ms) > a_bool->ms_switch)
    {
        a_bool->last_change_ms = a_now_ms;
        a_bool->value = !a_bool->value;
    }
    return a_bool->value;
}

// add all apps you want to use here
app_init_t app_inits[] =
{
    tetris_init_app,
    ammo_counter_init_app,
    snake_init_app
};

void switch_app(app_context_t* a_app, void (*a_init)(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx))
{
    if (a_app->close)
    {
        a_app->close(a_app);
        memory_arena_set_marker(&g_memory_arena, a_app->memory_arena_marker);
    }
    memory_set(a_app, 0, sizeof(a_app));
    a_init(a_app, &g_memory_arena, &g_render_ctx);
}

int main(void)
{
    stdio_init_all();

    uint32_t load_new_app_ms = 5000;
    uint32_t last_app_loaded = load_new_app_ms;
    uint16_t current_app = 0;

    render_load_func(&g_render_ctx, DRIVER_ST7789);
    render_init_context(&g_render_ctx, TEST_W, TEST_H, 0, 20, 100 * 1000);
    app_context_t app;
    memory_set(&app, 0, sizeof(app));

    button_context_t switch_app_button;
    button_init_context(&switch_app_button, 2, 30);
    switch_app(&app, app_inits[current_app++]);

    delay_bool_t led_bool = create_delay_bool_value(500, false);
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true)
    {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        button_update(&switch_app_button, now_ms);
        if (button_pressed(&switch_app_button))
        {
            printf("new app loaded");
            switch_app(&app, app_inits[current_app++]);
            last_app_loaded = now_ms;
            if (current_app >= sizeof(app_inits) / sizeof(app_init_t))
                current_app = 0;
        }
        
        if (app.update(&app, now_ms))
            app.render(&app, &g_render_ctx);

        gpio_put(PICO_DEFAULT_LED_PIN, get_delay_bool_value(&led_bool, now_ms));
    }
}
