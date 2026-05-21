#ifndef PWM_H
#define PWM_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

typedef struct pwm_context_h
{
    uint16_t pin;
    int dma_chan;
} pwm_context_h;

bool pwm_init_context(pwm_context_h* a_ctx);
bool pwm_close_context(pwm_context_h* a_ctx);
bool pwm_audio_play(pwm_context_h* a_ctx, const uint8_t* a_data, size_t a_len);
bool pwm_audio_stop(audio_context_t* ctx);
bool pwm_is_playing(const pwm_context_h* a_ctx);

#endif // PWM_H
