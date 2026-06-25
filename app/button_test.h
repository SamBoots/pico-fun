#ifndef BUTTON_TEST_H
#define BUTTON_TEST_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

void button_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params);
app_update_status_t button_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
void button_test_render(app_context_t* a_app, render_context_t* a_ctx);
void button_test_close(app_context_t* a_app);
void button_test_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count);
void button_test_default_params(uint8_t* a_param_buf, app_param_descriptor_t* a_descs);

#endif // BUTTON_TEST_H
