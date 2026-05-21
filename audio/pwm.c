#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pwm.h"

static bool s_pwm_is_playing = ;

bool pwm_init_context(pwm_context_h* a_ctx)
{
        gpio_set_function(a_pwm_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_AUDIO);
    psm_set_wrap(slice, 255);
    pwm_set_clkdiv(slice, 60.f);
    pwm_set_enabled(slice, true);

    a_ctx.pin = a_pwm_pin;
    a_ctx.dma_chan = dma_claim_unused_channel(true);
    a_ctx.is_playing = false;

    dma_channel_set_irq0_enabled(a_ctx.dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    return true;
}

bool pwm_close_context(pwm_context_h* a_ctx, uint16_t a_pwm_pin)
{

}

void pwm_audio_play(audio_context_t* ctx, const uint8_t* data, size_t len) {
    if (!data || len == 0) return;
    audio_stop(ctx);

    dma_channel_config c = dma_channel_get_default_config(ctx->dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c,  true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pwm_get_dreq(ctx->slice));

    ctx->is_playing = true;

    dma_channel_configure(
        ctx->dma_chan, &c,
        &pwm_hw->slice[ctx->slice].cc,
        data, len, true
    );
    return true;
}

void pwm_audio_stop(audio_context_t* ctx) {
    if (!ctx->is_playing) return false;
    dma_channel_abort(ctx->dma_chan);
    ctx->is_playing = false;
    return true;
}

bool pwm_is_playing(const pwm_context_h* a_ctx)
{
    return dma_channel_is_busy(ctx->dma_chan);
}
