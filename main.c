#define TEST_W 64
#define TEST_H 48
 
#include "pico/stdlib.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"

#include "memory/memory_arena.h"

#include "app/app.h"
#include "app/ammo_counter.h"
#include "app/snake.h"

#include "stdio.h"

static render_context_t g_render_ctx;
static memory_arena_t g_memory_arena;

// add all apps you want to use here
app_init_t app_inits[] =
{
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

    render_init_context(&g_render_ctx, TEST_W, TEST_H, 125);
    app_context_t app;
    memory_set(&app, 0, sizeof(app));

    while (true)
    {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        if ((uint32_t)(now_ms - last_app_loaded) > load_new_app_ms)
        {
            printf("new app loaded");
            switch_app(&app, app_inits[current_app++]);
            last_app_loaded = now_ms;
            if (current_app >= sizeof(app_inits) / sizeof(app_init_t))
                current_app = 0;
        }
        
        if (app.update(&app, now_ms))
            app.render(&app, &g_render_ctx);
    }
}
