#include "pico/stdlib.h"
#include "app.h"
#include "../io/nfc.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../memory/memory_arena.h"

typedef struct nfc_test_params_t
{
    uint8_t pad;
} nfc_test_params_t;

typedef struct nfc_test_context_t
{
    nfc_context_t nfc_rw;
    bool chip_detected;
} nfc_test_context_t;

static void nfc_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params)
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

static app_update_status_t nfc_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    if (nfc_test_ctx->chip_detected != nfc_detect_tag(&nfc_test_ctx->nfc_rw))
    {
        nfc_test_ctx->chip_detected = !nfc_test_ctx->chip_detected;
        return APP_RENDER;
    }
    return APP_OK;
}

static void nfc_test_render(app_context_t* a_app, render_context_t* a_ctx)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    if (nfc_test_ctx->chip_detected)
        render_fill(a_ctx, COLOR_RED);
    else
        render_fill(a_ctx, COLOR_BLUE);
}

static void nfc_test_close(app_context_t* a_app)
{

}

static void nfc_test_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count)
{
    *a_param_buf_size = sizeof(nfc_test_params_t);
    *a_desc_count = 1;
}

static void nfc_test_default_params(uint8_t* a_param_buf, app_param_descriptor_t* a_descs)
{
    nfc_test_params_t* defaults = (nfc_test_params_t*)a_param_buf;
    defaults->pad = 0;

    a_descs[0] = APP_PARAM("pad", PARAM_U8, offsetof(nfc_test_params_t, pad), 0, 1);
}

APP_REGISTER("nfc_test");
