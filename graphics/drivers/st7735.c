#include <stdlib.h>
#include "pico/stdlib.h"
#include "../render_types.h"
#include "st7789.h"
 
static inline void dc_low(const render_context_t* a_context)  { gpio_put(a_context->spi.pin_dc, 0); }
static inline void dc_high(const render_context_t* a_context) { gpio_put(a_context->spi.pin_dc, 1); }
static inline void cs_low(const render_context_t* a_context)  { gpio_put(a_context->spi.pin_cs, 0); }
static inline void cs_high(const render_context_t* a_context) { gpio_put(a_context->spi.pin_cs, 1); }


