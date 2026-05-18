#define TEST_W 64
#define TEST_H 48
 
#include "pico/stdlib.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"

#include "memory/memory_arena.h"

#include "app/app.h"
#include "app/ammo_counter.h"

static render_context_t g_render_ctx;
static memory_arena_t g_memory_arena;

int main(void)
{
    stdio_init_all();
    render_init_context(&g_render_ctx, TEST_W, TEST_H, 125);
    app_context_t app;
    ammo_counter_init_app(&app, &g_memory_arena, &g_render_ctx);
    while (true) 
    {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        if (app.update(&app, now_ms))
            app.render(&app, &g_render_ctx);
    }
}
