#include "pico/stdlib.h"
#include "app.h"
#include "nfc_test.h"
#include "../io/nfc.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../memory/memory_arena.h"

typedef struct nfc_test_context_t
{
    nfc_context_t nfc_rw;
    bool chip_detected;
} nfc_test_context_t;

void nfc_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(nfc_test_context_t));
    a_app->update = nfc_test_update;
    a_app->render = nfc_test_render;
    a_app->close = nfc_test_close;

    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    nfc_init_info_t nfc_init_info;
    nfc_init_info.pin_sda = 26;
    nfc_init_info.pin_scl = 27;
    nfc_init_info.i2c = i2c1;
    nfc_test_ctx->chip_detected = false;
    if (nfc_init(&nfc_test_ctx->nfc_rw, &nfc_init_info, DRIVER_PN532))
        render_fill(a_ctx, COLOR_BLUE);
    else
        render_fill(a_ctx, COLOR_RED);
}

app_update_status_t nfc_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    if (nfc_test_ctx->chip_detected != nfc_detect_tag(&nfc_test_ctx->nfc_rw))
    {
        nfc_test_ctx->chip_detected = !nfc_test_ctx->chip_detected;
        return APP_RENDER;
    }
    return APP_OK;
}

void nfc_test_render(app_context_t* a_app, render_context_t* a_ctx)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    if (nfc_test_ctx->chip_detected)
        render_fill(a_ctx, COLOR_RED);
    else
        render_fill(a_ctx, COLOR_BLUE);
}

void nfc_test_close(app_context_t* a_app)
{

}
