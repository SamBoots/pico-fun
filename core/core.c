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
#include "app_menu.h"

#define _LED_BLINK

#ifdef USE_FS
#include "io/fs.h"
#endif // USE_FS

#define TEST_H 240
#define TEST_W 280

typedef void (*update_loop_f)(u32 a_now_ms, app_context_t* a_app);

static render_context_t g_render_ctx;
static memory_arena_t g_memory_arena;
static update_loop_f g_update_loop;

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

void update_loop_menu(u32 a_now_ms, app_context_t* a_app);
void update_loop_app(u32 a_now_ms, app_context_t* a_app);

void update_loop_app(u32 a_now_ms, app_context_t* a_app)
{
    app_update_status_t update_status = a_app->update(a_app, &g_memory_arena, &g_render_ctx, a_now_ms);
    switch (update_status)
    {
    case APP_RENDER:
        a_app->render(a_app, &g_render_ctx);
        break;
    case APP_EXIT:
        g_update_loop = update_loop_menu;
        app_menu_init(&g_memory_arena, &g_render_ctx);
        break;
    }
}

void update_loop_menu(u32 a_now_ms, app_context_t* a_app)
{
    app_update_status_t update_status = app_menu_update(a_app, &g_memory_arena, &g_render_ctx, a_now_ms);
    switch (update_status)
    {
    case APP_RENDER:
        app_menu_render(&g_render_ctx);
        break;
    case APP_EXIT:
        g_update_loop = update_loop_app;
        app_menu_close();
        break;
    }
}

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
        u32 now_ms = to_ms_since_boot(get_absolute_time());
        button_update(&exit_app_0, now_ms);
        button_update(&exit_app_1, now_ms);

        if (button_held(&exit_app_0) && button_held(&exit_app_1))
        {
            switch_app(&app, app_select_init_app, NULL);
        }

        g_update_loop(now_ms, &app);

        #ifdef _LED_BLINK
        gpio_put(PICO_DEFAULT_LED_PIN, delay_bool_update(&led_bool, now_ms));
        #endif // _LED_BLINK
    }
}
