#ifndef PWM_H
#define PWM_H

typedef struct pwm_context_t
{
    uint8_t pin;
    uint8_t wrap_val;
    uint8_t level;
    uint8_t channel;
    uint8_t slice;
} pwm_context_t;

bool pwm_init_context(pwm_context_t* a_ctx, uint16_t a_pwm_pin, uint16_t a_wrap, uint16_t a_level);
bool pwm_close_context(pwm_context_t* a_ctx);
bool pwm_set_level(pwm_context_t* a_ctx, uint8_t a_level);

#endif // PWM_H
