#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "io_types.h"
#include "button.h"
#include "stdio.h"

int button_init_context(button_context_t* a_ctx, uint16_t a_pin, uint16_t a_debounce_ms)
{
    gpio_init(a_pin);  
    gpio_set_dir(a_pin,  GPIO_IN); 
    gpio_pull_up(a_pin);
    a_ctx->last_change_ms = 0;
    a_ctx->pin = a_pin;
    a_ctx->debounce_ms = a_debounce_ms;
    a_ctx->previous = false;
    a_ctx->current = false;
    return 1;
}

int button_free_context(button_context_t* a_ctx)
{
    gpio_deinit(a_ctx->pin);
}

int button_update(button_context_t* a_ctx, uint32_t a_now_ms)
{
    if ((uint32_t)(a_now_ms - a_ctx->last_change_ms) > a_ctx->debounce_ms)
    {
        a_ctx->last_change_ms = a_now_ms;
        a_ctx->previous = a_ctx->current;
        a_ctx->current = gpio_get(a_ctx->pin);
        return 1;
    }
    return 0;
}

bool button_pressed(const button_context_t* a_ctx)
{
    return !a_ctx->current && a_ctx->previous;
}

bool button_released(const button_context_t* a_ctx)
{
    return a_ctx->current && !a_ctx->previous;
}

bool button_held(const button_context_t* a_ctx)
{
    return !a_ctx->current && !a_ctx->previous;
}
