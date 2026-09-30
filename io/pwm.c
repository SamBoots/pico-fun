#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pwm.h"

bool pwm_init_context(pwm_context_t* a_ctx, uint16_t a_pwm_pin, uint16_t a_wrap, uint16_t a_level)
{
    gpio_set_function(a_pwm_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(a_pwm_pin);
    uint channel = pwm_gpio_to_channel(a_pwm_pin);
    pwm_set_wrap(slice, a_wrap);
    pwm_set_clkdiv_int_frac(slice, 60, 0);
    pwm_set_chan_level(slice, channel, a_level);
    pwm_set_enabled(slice, true);

    a_ctx->slice = slice;
    a_ctx->channel = channel;
    a_ctx->wrap = a_wrap;
    a_ctx->level = a_level;
    a_ctx->pin = a_pwm_pin;
    return true;
}

bool pwm_close_context(pwm_context_t* a_ctx)
{
    pwm_set_chan_level(a_ctx->slice, a_ctx->channel, 0);
    pwm_set_enabled(a_ctx->slice, false);
    gpio_set_function(a_ctx->pin, GPIO_FUNC_SIO);
    gpio_set_dir(a_ctx->pin, GPIO_OUT);
    gpio_put(a_ctx->pin, 0);
    return true;
}

bool pwm_set_level(pwm_context_t* a_ctx, uint8_t a_level)
{
    pwm_set_chan_level(a_ctx->slice, a_ctx->channel, a_level);
    return true;
}

bool pwm_set_duty_percent(pwm_context_t* a_ctx, uint8_t  a_percent)
{
    if (a_percent > 100) a_percent = 100;
    return pwm_set_level(a_ctx, (uint8_t)((uint16_t)a_percent * 255 / 100));
}
