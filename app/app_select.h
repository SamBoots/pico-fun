#ifndef APP_SELECT_H
#define APP_SELECT_H

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;
typedef struct app_context_t app_context_t;

typedef struct app_select_params_t
{
    app_init_t* app_inits;
    uint32_t app_count;
} app_select_params_t;

void app_select_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params);
app_update_status_t app_select_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
void app_select_render(app_context_t* a_app, render_context_t* a_ctx);
void app_select_close(app_context_t* a_app);

#endif // APP_SELECT_H
