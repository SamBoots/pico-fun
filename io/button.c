#include "core/types.h"
#include "os/os.h"
#include "button.h"

bool button_init_context(button_context_t* a_ctx, u8 a_pin, u16 a_debounce_ms)
{
    a_ctx->last_change_ms = 0;
    a_ctx->pin = a_pin;
    a_ctx->debounce_ms = a_debounce_ms;
    a_ctx->previous = false;
    a_ctx->current = false;
    ERROR_TYPE_T val = os_gpio_init(a_pin, OS_GPIO_MODE_IN, true);
    os_pin_high(a_pin);

    return val == ERROR_TYPE_OK;
}

bool button_free_context(button_context_t* a_ctx)
{
    return os_gpio_deinit(a_ctx->pin) == ERROR_TYPE_OK;
}

bool button_update(button_context_t* a_ctx, u32 a_now_ms)
{
    if ((u32)(a_now_ms - a_ctx->last_change_ms) > a_ctx->debounce_ms)
    {
        a_ctx->last_change_ms = a_now_ms;
        a_ctx->previous = a_ctx->current;
        a_ctx->current = os_pin_read(a_ctx->pin);
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

bool button_unheld(const button_context_t* a_ctx)
{
    return a_ctx->current && a_ctx->previous;
}
