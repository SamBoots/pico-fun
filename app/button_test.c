#include "pico/stdlib.h"
#include "app.h"
#include "button_test.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"
#include "math.h"

typedef struct button_test_context_t
{
    button_context_t button_0;
    button_context_t button_1;
    button_context_t button_2;
    button_context_t button_3;
} button_test_context_t;

// simple sine wave tone, 4000 samples at 8000Hz = 0.5 seconds
static uint8_t s_tone[4000];

void button_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(button_test_context_t));
    a_app->update = button_test_update;
    a_app->render = button_test_render;
    a_app->close = button_test_close;
    

    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
}

app_update_status_t button_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
    

    // never render
    return APP_OK;
}

void button_test_render(app_context_t* a_app, render_context_t* a_ctx)
{
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
}

void button_test_close(app_context_t* a_app)
{
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
}
