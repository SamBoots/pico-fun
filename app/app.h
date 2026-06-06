#ifndef APP_H
#define APP_H

typedef struct render_context_t render_context_t;
typedef struct memory_arena_t memory_arena_t;

typedef enum { APP_OK, APP_ERR, APP_RENDER, APP_EXIT, APP_EXIT_NO_CLOSE } app_update_status_t;

typedef struct app_context_t
{
    uint32_t memory_arena_marker; // start of the app's memory
    uint8_t* user_data;
    app_update_status_t (*update)(struct app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
    void (*render)(struct app_context_t* a_app, render_context_t* a_ctx);
    void (*close)(struct app_context_t* a_app);
} app_context_t;

typedef void (*app_init_t)(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params);

#endif // APP_H
