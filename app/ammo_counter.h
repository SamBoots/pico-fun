#ifndef AMMO_COUNTER_H
#define AMMO_COUNTER_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

void ammo_counter_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx);
bool ammo_counter_update(app_context_t* a_app, uint32_t a_now_ms);
void ammo_counter_render(app_context_t* a_app, render_context_t* a_ctx);
void ammo_counter_close(app_context_t* a_app);

#endif // AMMO_COUNTER_H