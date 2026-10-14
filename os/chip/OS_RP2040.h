#ifndef OS_RP2040
#define OS_RP2040

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define OS_INLINE static inline __attribute__((always_inline))

/* ======================= Capability flags ======================= */
#define OS_HAS_CORE_VOLTAGE_CTRL   1   // vreg_set_voltage() available
#define OS_HAS_PLL_BYPASS          1   // can run sys clock straight from XOSC
#define OS_HAS_DORMANT             1   // dormant (all oscillators off) mode
#define OS_HAS_SMPS_PIN            1   // GPIO23 PS pin (plain Pico only; 0 on W)
#define OS_PWM_SLICE_COUNT         8
#define OS_PWM_WRAP_MAX            65535u

/* ======================= Storage ======================= */
#define OS_FLASH_SECTOR_BYTES      4096u
#define OS_SETTINGS_MAGIC          0x4F535031u   // "OSP1"

/* ======================= Clocks ======================= */
#define OS_XOSC_KHZ                12000u   // crystal on Pico boards
#define OS_USB_PLL_KHZ             48000u   // fixed, used for USB and peripherals
 
#define OS_SYS_CLK_DEFAULT_KHZ     125000u  // SDK default
#define OS_SYS_CLK_MAX_KHZ         125000u
#define OS_SYS_CLK_MIN_PLL_KHZ     16000u   // practical PLL floor (VCO >= 750MHz)
#define OS_SYS_CLK_ULTRA_LOW_KHZ   12000u   // run directly from crystal
#define OS_SYS_CLK_STEP_KHZ        1000u    // fine-grained slider step
 
/* Named presets (kHz) */
#define OS_PRESET_ECO_KHZ          48000u
#define OS_PRESET_BALANCED_KHZ     90000u
#define OS_PRESET_PERF_KHZ         125000u

/* ======================= Pins (PLACEHOLDERS) ======================= */
#define OS_PIN_BL0                 22
#define OS_PIN_BL1                 7
 
#define OS_SPI0                    spi0
#define OS_SPI0_PIN_CLK            18
#define OS_SPI0_PIN_DIN            19
#define OS_SPI0_PIN_CS             17
#define OS_SPI0_PIN_DC             20
#define OS_SPI0_PIN_RST            21

#define OS_SPI1                    spi1
#define OS_SPI1_PIN_CLK            10
#define OS_SPI1_PIN_DIN            11
#define OS_SPI1_PIN_CS             13
#define OS_SPI1_PIN_DC             14
#define OS_SPI1_PIN_RST            15

/* ======================= inline function ======================= */

OS_INLINE void os_pin_low(u8 a_pin) { gpio_put(a_pin, 0); }
OS_INLINE void os_pin_high(u8 a_pin) { gpio_put(a_pin, 1); }
OS_INLINE bool os_pin_read(u8 a_pin) { return gpio_get(a_pin); }

OS_INLINE u32 os_millis(void) { return to_ms_since_boot(get_absolute_time()); }
OS_INLINE u32 os_micros32(void) { return time_us_32(); }

#endif // OS_RP2040
