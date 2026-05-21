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

void pwm_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{

}

bool pwm_test_update(app_context_t* a_app, uint32_t a_now_ms)
{

}

void pwm_test_render(app_context_t* a_app, render_context_t* a_ctx)
{

}

void pwm_test_close(app_context_t* a_app)
{

}
