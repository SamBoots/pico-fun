#include "types.h"
#include "common.h"
#include "core.h"
#include "app/app.h"
#include "os/OS.h"
#include "memory_arena.h"
#include "io/button.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"

// menu stuff, might change this
#include "app_select.h"

#define _LED_BLINK

#ifdef USE_FS
#include "io/fs.h"
#endif // USE_FS

#define TEST_H 240
#define TEST_W 280
static render_context_t g_render_ctx;
static memory_arena_t g_memory_arena;

static void switch_app(app_context_t* a_app, app_init_t a_init, void* a_params)
{
    if (a_app->close)
    {
        a_app->close(a_app);
        memory_arena_set_marker(&g_memory_arena, a_app->memory_arena_marker);
    }
    memory_set(a_app, 0, sizeof(a_app));
    a_init(a_app, &g_memory_arena, &g_render_ctx, a_params);
}

typedef struct core_context_t
{
    bool value;
    u32 last_change_ms;
    u32 ms_switch;
} core_context_t;

void update_loop_core(void)
{
    stdio_init_all();
    u32 load_new_app_ms = 5000;
    u32 last_app_loaded = load_new_app_ms;

    render_load_func(&g_render_ctx, DRIVER_ST7789);
    render_init_context(&g_render_ctx, TEST_W, TEST_H, 20, 0, 0);
    app_context_t app;
    memory_set(&app, 0, sizeof(app));
    switch_app(&app, app_select_init_app, NULL);

    #ifdef _LED_BLINK
    delay_bool_t led_bool = delay_bool_init(500, false);
    os_gpio_init(PICO_DEFAULT_LED_PIN, OS_GPIO_MODE_OUT, false);
    #endif // _LED_BLINK

    button_context_t exit_app_0;
    button_context_t exit_app_1;
    button_init_context(&exit_app_0, 2, 10);
    button_init_context(&exit_app_1, 3, 10);

    #ifdef USE_FS
    fs_init();
    #endif // USE_FS

    while (true)
    {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        button_update(&exit_app_0, now_ms);
        button_update(&exit_app_1, now_ms);

        if (button_held(&exit_app_0) && button_held(&exit_app_1))
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

        #ifdef _LED_BLINK
        gpio_put(PICO_DEFAULT_LED_PIN, delay_bool_update(&led_bool, now_ms));
        #endif // _LED_BLINK
    }
}
