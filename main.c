#define TEST_W 240
#define TEST_H 280
 
#include "pico/stdlib.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"

#include "io/io_types.h"
#include "io/button.h"
#include "io/fs.h"

#include "memory/memory_arena.h"

#include "app/app.h"
#include "app/app_select.h"

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

void switch_app(app_context_t* a_app, app_init_t a_init, void* a_params)
{
    if (a_app->close)
    {
        a_app->close(a_app);
        memory_arena_set_marker(&g_memory_arena, a_app->memory_arena_marker);
    }
    memory_set(a_app, 0, sizeof(a_app));
    a_init(a_app, &g_memory_arena, &g_render_ctx, a_params);
}

int main(void)
{
    stdio_init_all();

    uint32_t load_new_app_ms = 5000;
    uint32_t last_app_loaded = load_new_app_ms;

    render_load_func(&g_render_ctx, DRIVER_ST7789);
    render_init_context(&g_render_ctx, TEST_W, TEST_H, 0, 20, 10 * 1000);
    app_context_t app;
    memory_set(&app, 0, sizeof(app));
    switch_app(&app, app_select_init_app, NULL);

    delay_bool_t led_bool = create_delay_bool_value(500, false);
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    button_context_t exit_app;
    button_init_context(&exit_app, 13, 10);

    fs_init();
    while (true)
    {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        button_update(&exit_app, now_ms);

        if (button_pressed(&exit_app))
        {
            switch_app(&app, app_select_init_app, NULL);
        }

        app_update_status_t update_status = app.update(&app, &g_memory_arena, &g_render_ctx, now_ms);
        switch (update_status)
        {
        case APP_RENDER:
            app.render(&app, &g_render_ctx);
            break;
        case APP_EXIT:
            switch_app(&app, app_select_init_app, NULL);
            break;
        }

        gpio_put(PICO_DEFAULT_LED_PIN, get_delay_bool_value(&led_bool, now_ms));
    }
}
