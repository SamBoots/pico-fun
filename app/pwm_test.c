#include "pico/stdlib.h"
#include "app.h"
#include "pwm_test.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"
#include "../audio/pwm.h"
#include "math.h"

typedef struct pwm_test_context_t
{
    pwm_context_t pwm;
} pwm_test_context_t;

// simple sine wave tone, 4000 samples at 8000Hz = 0.5 seconds
static uint8_t s_tone[4000];

void pwm_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(pwm_test_context_t));
    a_app->update = pwm_test_update;
    a_app->render = pwm_test_render;
    a_app->close = pwm_test_close;
    
    pwm_test_context_t* app_ctx = (pwm_test_context_t*)a_app->user_data;
    pwm_init_context(&app_ctx->pwm, 13);

    for (int i = 0; i < 4000; i++)
        s_tone[i] = (uint8_t)(128 + 127 * sinf(2.0f * 3.14159f * 440.0f * i / 8000.0f));
    pwm_audio_play(&app_ctx->pwm, s_tone, sizeof(s_tone));
}

app_update_status_t pwm_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    pwm_test_context_t* app_ctx = (pwm_test_context_t*)a_app->user_data;
    if (!pwm_is_playing(&app_ctx->pwm))
    {
        pwm_audio_play(&app_ctx->pwm, s_tone, sizeof(s_tone));
    }

    // never render
    return APP_OK;
}

void pwm_test_render(app_context_t* a_app, render_context_t* a_ctx)
{
    
}

void pwm_test_close(app_context_t* a_app)
{

}
