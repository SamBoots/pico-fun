#ifndef APP_H
#define APP_H

typedef struct render_context_t render_context_t;
typedef struct memory_arena_t memory_arena_t;
typedef struct app_context_t app_context_t;

typedef enum { APP_OK, APP_ERR, APP_RENDER, APP_EXIT, APP_EXIT_NO_CLOSE } app_update_status_t;

typedef enum { PARAM_U16, PARAM_U8, PARAM_BOOL } param_type_t;

typedef struct app_param_descriptor_t
{
    const char* label;
    uint16_t label_len;
    param_type_t type;
    uint16_t byte_offset;
    uint16_t min, max;
} app_param_descriptor_t;

#define APP_PARAM(a_label, a_type, a_offsetoff, a_min, a_max)\
    ((app_param_descriptor_t)                                \
    {                                                        \
        .label       = a_label,                              \
        .label_len   = sizeof(a_label) - 1,                  \
        .type        = a_type,                               \
        .byte_offset = a_offsetoff,                          \
        .min         = a_min,                                \
        .max         = a_max                                 \
    })

typedef void (*app_init_t)(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params);
typedef app_update_status_t (*app_update_t)(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms);
typedef void (*app_render_t)(app_context_t* a_app, render_context_t* a_ctx);
typedef void (*app_close_t)(app_context_t* a_app);
typedef void (*app_default_sizes)(size_t* a_param_buf_size, size_t* a_desc_count);
typedef void (*app_default_params)(uint8_t* a_param_buf, app_param_descriptor_t* a_descs);

typedef struct app_entry_t
{
    const char* name;
    int16_t name_len;
    app_init_t init;
    app_update_t update;
    app_render_t render;
    app_close_t close;
    app_default_sizes default_sizes;
    app_default_params default_params;
} app_entry_t;

typedef struct app_context_t
{
    uint32_t memory_arena_marker; // start of the app's memory
    uint8_t* user_data;
    app_update_t update;
    app_render_t render;
    app_close_t close;
} app_context_t;

#define STRLEN(s) (sizeof(s) - 1)
#define APP_REGISTER(_name)                             \
    __attribute__((section("app_entries"), used))       \
    static const app_entry_t _name##_descriptor = {     \
        .name           = #_name,                       \
        .name_len       = sizeof(#_name) - 1,           \
        .init           = _name##_init_app,                 \
        .update         = _name##_update,               \
        .render         = _name##_render,               \
        .close          = _name##_close,                \
        .default_sizes  = _name##_default_sizes,        \
        .default_params = _name##_default_params        \
    };

extern app_entry_t __start_app_entries;
extern app_entry_t __stop_app_entries;

#endif // APP_H
