#ifndef APP_H
#define APP_H

typedef struct render_context_t render_context_t;

typedef struct app_context_t
{
    uint32_t memory_arena_marker; // start of the app's memory
    uint8_t* user_data;
    bool (*update)(struct app_context_t* a_app, uint32_t a_now_ms);
    void (*render)(struct app_context_t* a_app, render_context_t* a_ctx);
    void (*close)(struct app_context_t* a_app);
} app_context_t;

#endif // APP_H
