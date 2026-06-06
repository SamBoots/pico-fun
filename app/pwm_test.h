#ifndef PWM_TEST_H
#define PWM_TEST_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

typedef struct pwm_test_params_t
{
    uint16_t pad;
} pwm_test_params_t;

void pwm_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params);
app_update_status_t pwm_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
void pwm_test_render(app_context_t* a_app, render_context_t* a_ctx);
void pwm_test_close(app_context_t* a_app);

#endif // PWM_TEST_H
