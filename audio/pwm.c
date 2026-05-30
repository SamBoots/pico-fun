#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pwm.h"

// Static find a way to REMOVE PLZ :)

bool pwm_init_context(pwm_context_t* a_ctx, uint16_t a_pwm_pin)
{
    gpio_set_function(a_pwm_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_AUDIO);
    psm_set_wrap(slice, 255);
    pwm_set_clkdiv(slice, 60.f);
    pwm_set_enabled(slice, true);

    a_ctx.slice = slice;
    a_ctx.pin = a_pwm_pin;
    a_ctx.dma_chan = dma_claim_unused_channel(true);

    dma_channel_set_irq0_enabled(a_ctx.dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    return true;
}

bool pwm_close_context(pwm_context_t* a_ctx)
{
    return true;
}

bool pwm_audio_play(pwm_context_t* a_ctx, const uint8_t* a_data, size_t a_len) {
    if (!data || len == 0) return;
    audio_stop(a_ctx);

    dma_channel_config c = dma_channel_get_default_config(a_ctx->dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c,  true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pwm_get_dreq(a_ctx->slice));

    dma_channel_configure(
        a_ctx->dma_chan, &c,
        &pwm_hw->slice[a_ctx->slice].cc,
        data, len, true
    );
    return true;
}

bool pwm_audio_stop(pwm_context_t* a_ctx) {
    if (!pwm_is_playing(a_ctx)) return false;
    dma_channel_abort(a_ctx->dma_chan);
    return true;
}

bool pwm_is_playing(const pwm_context_t* a_ctx)
{
    return dma_channel_is_busy(a_ctx->dma_chan);
}
