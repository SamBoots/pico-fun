#ifndef OS_H
#define OS_H

//#if RP_2040
#include "chip/OS_RP2040.h"
//#endif // RP_2040

// hardware stuff here for now
#include "hardware/dma.h"
#include "hardware/spi.h"

typedef enum OS_POWER_MODE_T
{
    OS_POWER_MODE_PERF,
    OS_POWER_MODE_NORMAL,
    OS_POWER_MODE_ECO
} OS_POWER_MODE_T;

typedef enum OS_GPIO_MODE_T
{
    OS_GPIO_MODE_IN,
    OS_GPIO_MODE_OUT,
    OS_GPIO_MODE_I2C,
    OS_GPIO_MODE_SPI,
    OS_GPIO_MODE_PWM,
    OS_GPIO_MODE_NONE,
} OS_GPIO_MODE_T;

typedef struct pwm_context_t
{
    u8 pin;
    u8 wrap;
    u32 channel;
    u32 slice;
} pwm_context_t;

ERROR_TYPE_T os_init_context(void);
ERROR_TYPE_T os_set_cpu_mhz(OS_POWER_MODE_T a_power_mode);
u32 os_sys_hz(void);
u32 os_peri_hz(void);

ERROR_TYPE_T os_gpio_init(u8 a_pin, OS_GPIO_MODE_T a_mode, bool a_pull_up);
ERROR_TYPE_T os_gpio_deinit(u8 a_pin);
void os_gpio_write(u8 a_pin, bool a_level);
bool os_gpio_read(u8 a_pin);
void os_delay_us(u32 a_us);

ERROR_TYPE_T os_init_pwm(pwm_context_t* a_ctx, u8 a_pin, u32 a_freq_hz, u16 a_duty_permille);
ERROR_TYPE_T os_free_pwm(pwm_context_t* a_ctx);

#endif // OS_H
