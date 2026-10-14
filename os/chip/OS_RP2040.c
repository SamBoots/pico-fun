#include "defines/types.h"
#include "os/OS.h"
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

typedef struct os_context_t
{
    u32 cpu_khz;
    OS_POWER_MODE_T power_mode;
} os_context_t;

static os_context_t s_os_context;

ERROR_TYPE_T os_init_context(void)
{
    s_os_context.cpu_khz = OS_PRESET_PERF_KHZ;
    s_os_context.power_mode = OS_POWER_MODE_PERF;
    return ERROR_TYPE_OK;
}

ERROR_TYPE_T os_set_cpu_mhz(OS_POWER_MODE_T a_power_mode)
{
    usize khz;
    usize voltage;

    switch (a_power_mode)
    {
    case OS_POWER_MODE_PERF:
        khz = OS_PRESET_PERF_KHZ;
        voltage = VREG_VOLTAGE_1_10;
        break;
    case OS_POWER_MODE_NORMAL:
        khz = OS_PRESET_BALANCED_KHZ;
        voltage = VREG_VOLTAGE_1_00;
        break;
    case OS_POWER_MODE_ECO:
        khz = OS_PRESET_ECO_KHZ;
        voltage = VREG_VOLTAGE_0_90;
        break;
    }

    usize cur_khz = clock_get_hz(clk_sys) / 1000;

    bool success;
    // when new power is higher then the current power setting
    if (khz > cur_khz)
    {
        vreg_set_voltage(voltage);
        sleep_ms(1);
        success = set_sys_clock_khz(khz, false);
    }
    else
    {
        success = set_sys_clock_khz(khz, false);
        if (success)
            vreg_set_voltage(voltage);
    }

    if (!success)
        return ERROR_TYPE_UNSUPPORTED;
        
    s_os_context.cpu_khz = khz;
    s_os_context.power_mode = a_power_mode;
    return ERROR_TYPE_OK;
}

u32 os_sys_hz(void)  { return clock_get_hz(clk_sys); }
u32 os_peri_hz(void) { return clock_get_hz(clk_peri); }

ERROR_TYPE_T os_gpio_init(u8 a_pin, OS_GPIO_MODE_T a_mode, bool a_pull_up)
{
    switch (a_mode)
    {
        case OS_GPIO_MODE_IN:
            gpio_init(a_pin);
            break;
        case OS_GPIO_MODE_OUT:
            gpio_init(a_pin);
            gpio_set_dir(a_pin, GPIO_OUT);
            break;
        case OS_GPIO_MODE_I2C:  gpio_set_function(a_pin, GPIO_FUNC_I2C);  break;
        case OS_GPIO_MODE_SPI:  gpio_set_function(a_pin, GPIO_FUNC_SPI);  break;
        case OS_GPIO_MODE_PWM:  gpio_set_function(a_pin, GPIO_FUNC_PWM);  break;
        case OS_GPIO_MODE_NONE: gpio_set_function(a_pin, GPIO_FUNC_NULL); break;
        default: return ERROR_TYPE_UNSUPPORTED;
    }

    if (a_pull_up)
        gpio_pull_up(a_pin);
    else
        gpio_disable_pulls(a_pin);

    return ERROR_TYPE_OK;
}

ERROR_TYPE_T os_gpio_deinit(u8 a_pin)
{
    gpio_set_dir(a_pin, GPIO_IN);
    gpio_disable_pulls(a_pin);
    gpio_set_function(a_pin, GPIO_FUNC_NULL);
    gpio_set_input_enabled(a_pin, false);
    return ERROR_TYPE_OK;
}

void os_gpio_write(u8 a_pin, bool a_level)
{
    gpio_put(a_pin, a_level);
}

bool os_gpio_read(u8 a_pin) { return gpio_get(a_pin); }
void os_delay_us(u32 a_us)  { sleep_us(a_us); }

ERROR_TYPE_T os_init_pwm(pwm_context_t* a_ctx, u8 a_pin, u32 a_freq_hz, u16 a_duty_permille)
{
    if (a_freq_hz == 0 || a_duty_permille > 1000) return ERROR_TYPE_UNSUPPORTED;

    u32 slice = pwm_gpio_to_slice_num(a_pin);
    u32 channel = pwm_gpio_to_channel(a_pin);

    u32 wrap = 999; // 0.1% resolution
    float div = (float)os_sys_hz() / ((float)a_freq_hz * (wrap + 1));
    if (div < 1.0f) {
        wrap = os_sys_hz() / a_freq_hz - 1;
        if (wrap < 99) return ERROR_TYPE_UNSUPPORTED; // under ~1% resolution
        div = 1.0f;
    }
    if (div > 255.9f) return ERROR_TYPE_UNSUPPORTED; // too slow

    gpio_set_function(a_pin, GPIO_FUNC_PWM);
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, (u16)wrap);
    pwm_set_chan_level(slice, channel, (u16)((wrap + 1) * a_duty_permille / 1000));
    pwm_set_enabled(slice, true);

    a_ctx->slice = slice;
    a_ctx->channel = channel;
    a_ctx->wrap = wrap;
    a_ctx->pin = a_pin;

    return ERROR_TYPE_OK;
}

ERROR_TYPE_T os_free_pwm(pwm_context_t* a_ctx)
{
    pwm_set_chan_level(a_ctx->slice, a_ctx->channel, 0);
    pwm_set_enabled(a_ctx->slice, false);
    gpio_set_function(a_ctx->pin, GPIO_FUNC_SIO);
    gpio_set_dir(a_ctx->pin, GPIO_OUT);
    gpio_put(a_ctx->pin, 0);
    return ERROR_TYPE_OK;
}

ERROR_TYPE_T os_set_pwn_level(u32 a_slice, u32 a_channel, u16 a_level)
{
    pwm_set_chan_level(a_slice, a_channel, a_level);
    return ERROR_TYPE_OK;
}
