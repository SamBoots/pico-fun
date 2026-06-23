#ifndef NFC_TEST_H
#define NFC_TEST_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

typedef struct nfc_test_params_t
{
    uint8_t pad;
} nfc_test_params_t;

void nfc_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params);
app_update_status_t nfc_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
void nfc_test_render(app_context_t* a_app, render_context_t* a_ctx);
void nfc_test_close(app_context_t* a_app);

#endif // NFC_TEST_H
