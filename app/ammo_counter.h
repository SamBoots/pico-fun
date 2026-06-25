#ifndef AMMO_COUNTER_H
#define AMMO_COUNTER_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

void ammo_counter_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params);
app_update_status_t ammo_counter_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
void ammo_counter_render(app_context_t* a_app, render_context_t* a_ctx);
void ammo_counter_close(app_context_t* a_app);
void ammo_counter_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count);
void ammo_counter_default_params(uint8_t* a_param_buf, app_param_descriptor_t* a_descs);

#endif // AMMO_COUNTER_H