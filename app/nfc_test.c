#include "pico/stdlib.h"
#include "app.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../io/nfc.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "../memory/memory_arena.h"

// replace these with own solution
#include <stdio.h>
#include <string.h>

typedef struct nfc_test_params_t
{
    uint8_t pad;
} nfc_test_params_t;

typedef struct nfc_test_context_t
{
    nfc_context_t nfc_rw;
    button_context_t button_next_page;
    int current_page;
    uint8_t page_increment;
    uint8_t nfc_data[16];   // for now 16 due to the nfc tags having a page increment of 4
} nfc_test_context_t;

static app_update_status_t nfc_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;

    button_update(&nfc_test_ctx->button_next_page, 100);

    if (nfc_test_ctx->current_page == -4 && button_pressed(&nfc_test_ctx->button_next_page))
    {
        if (nfc_read(&nfc_test_ctx->nfc_rw, nfc_test_ctx->current_page + 4, nfc_test_ctx->nfc_data, sizeof(nfc_test_ctx->nfc_data)))
            nfc_test_ctx->current_page += 4;
        else
            nfc_test_ctx->current_page = -4;
        return APP_RENDER;
    }
    return APP_OK;
}

const char* page_label(uint8_t page) {
    if (page <= 1) return "UID/BCC";
    if (page == 2) return "Static lock";
    if (page == 3) return "CC";
    if (page <= 129) return "User data";
    if (page == 130) return "Dynamic lock";
    if (page == 131) return "CFG0";
    if (page == 132) return "CFG1";
    if (page == 133) return "PWD";
    if (page == 134) return "PACK";
    return "?";
}

static void nfc_render(app_context_t* a_app, render_context_t* a_ctx)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    if (nfc_test_ctx->current_page != -4)
    {
        render_fill(a_ctx, COLOR_BLUE);
        for (int i = 0; i < nfc_test_ctx->page_increment; i++) {
            uint8_t page = nfc_test_ctx->current_page + i;
            const uint8_t *p = &nfc_test_ctx->nfc_data[i * 4];

            char ascii[5];
            for (int j = 0; j < 4; j++) {
                char c = p[j];
                if (c >= 'A' && c <= 'Z') {
                    c = c - 'A' + 'a';   // fold uppercase down to lowercase for display
                }
                ascii[j] = (c >= 0x20 && c < 0x7F) ? c : '.';
            }
            ascii[4] = '\0';
            char byte_text[40];
            snprintf(byte_text, strlen(byte_text),
                    "Page %3d [%-11s]: %02x %02x %02x %02x  | %s",
                    page, page_label(page), p[0], p[1], p[2], p[3], ascii);
            
            render_8x16glyphs(a_ctx, byte_text, sizeof(byte_text), 
            2, 2, COLOR_GREEN, COLOR_BLUE, 20, a_ctx->height / nfc_test_ctx->page_increment * i);
        }
    }
    else
        render_fill(a_ctx, COLOR_RED);
}

static void nfc_close(app_context_t* a_app)
{
    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    nfc_deinit(&nfc_test_ctx->nfc_rw);
}

static void nfc_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count)
{
    *a_param_buf_size = sizeof(nfc_test_params_t);
    *a_desc_count = 1;
}

static void nfc_default_params(uint8_t* a_param_buf, app_param_descriptor_t* a_descs)
{
    nfc_test_params_t* defaults = (nfc_test_params_t*)a_param_buf;
    defaults->pad = 0;

    a_descs[0] = APP_PARAM("pad", PARAM_U8, offsetof(nfc_test_params_t, pad), 0, 1);
}

static void nfc_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(nfc_test_context_t));
    a_app->update = nfc_update;
    a_app->render = nfc_render;
    a_app->close = nfc_close;

    nfc_test_context_t* nfc_test_ctx = (nfc_test_context_t*)a_app->user_data;
    nfc_init_info_t nfc_init_info;
    nfc_init_info.pin_sda = 8;
    nfc_init_info.pin_scl = 9;
    nfc_init_info.i2c = i2c0;
    nfc_test_ctx->current_page = -4;
    nfc_test_ctx->page_increment = 4;
    if (nfc_init(&nfc_test_ctx->nfc_rw, &nfc_init_info, DRIVER_PN532))
        render_fill(a_ctx, COLOR_BLUE);
    else
        render_fill(a_ctx, COLOR_RED);

    button_init_context(&nfc_test_ctx->button_next_page, 2, 10);
}

APP_REGISTER(nfc);
