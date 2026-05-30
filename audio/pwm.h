#ifndef PWM_H
#define PWM_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

typedef struct pwm_context_t
{
    uint16_t pin;
    int dma_chan;
    uint slice;
} pwm_context_t;

bool pwm_init_context(pwm_context_t* a_ctx,  uint16_t a_pwm_pin);
bool pwm_close_context(pwm_context_t* a_ctx);
bool pwm_audio_play(pwm_context_t* a_ctx, const uint8_t* a_data, size_t a_len);
bool pwm_audio_stop(pwm_context_t* ctx);
bool pwm_is_playing(const pwm_context_t* a_ctx);

#endif // PWM_H
