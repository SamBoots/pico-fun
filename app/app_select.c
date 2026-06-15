#include "pico/stdlib.h"
#include "app.h"
#include "app_select.h"
#include "ammo_counter.h"
#include "snake.h"
#include "pwm_test.h"
#include "nfc_test.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../io/fs.h"
#include "../memory/memory_arena.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "math.h"
#include "string.h"

typedef enum { PARAM_U16, PARAM_U8, PARAM_BOOL } param_type_t;
typedef enum { CURSOR_TYPE_APP = 0, CURSOR_TYPE_PARAM = 1, CURSOR_TYPE_PARAM_VAL = 2, CURSOR_TYPE_MAX = 3 } cursor_type_t;

typedef struct app_param_descriptor_t
{
    const char* label;
    uint16_t label_len;
    param_type_t type;
    uint16_t byte_offset;
    uint16_t min, max;
} app_param_descriptor_t;

typedef struct app_descriptor_t
{
    const char* name;
    uint16_t name_len;
    app_init_t init;
    const void* params;
    size_t params_size;
    const app_param_descriptor_t* descriptors;
    uint8_t descriptor_count;
} app_descriptor_t;

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define APP_PARAM(a_label, a_type, a_offsetoff, a_min, a_max)\
    {                                                        \
        .label       = a_label,                              \
        .label_len   = sizeof(a_label) - 1,                  \
        .type        = a_type,                               \
        .byte_offset = a_offsetoff,                          \
        .min         = a_min,                                \
        .max         = a_max                                 \
    }

#define APP_ENTRY(a_name, a_prefix)                              \
    {                                                            \
        .name             = a_name,                              \
        .name_len         = sizeof(a_name) - 1,                  \
        .init             = a_prefix##_init_app,                 \
        .params           = &a_prefix##_defaults,                \
        .params_size      = sizeof(a_prefix##_defaults),         \
        .descriptors      = a_prefix##_descriptors,              \
        .descriptor_count = ARRAY_LENGTH(a_prefix##_descriptors) \
    }

static const app_param_descriptor_t ammo_counter_descriptors[] = 
{
    APP_PARAM("maxammo", PARAM_U8, offsetof(ammo_counter_params_t, max_ammo), 1, 99),
    APP_PARAM("scale", PARAM_U8, offsetof(ammo_counter_params_t, scale), 1, 16)
};

static const app_param_descriptor_t snake_descriptors[] = 
{
    APP_PARAM("scale", PARAM_U8, offsetof(snake_params_t, scale), 1, 17)
};

static const app_param_descriptor_t pwm_test_descriptors[] = 
{
    APP_PARAM("pad", PARAM_U8, offsetof(pwm_test_params_t, pad), 1, 17)
};

static const app_param_descriptor_t nfc_test_descriptors[] = 
{
    APP_PARAM("pad", PARAM_U8, offsetof(nfc_test_params_t, pad), 1, 17)
};

static const ammo_counter_params_t ammo_counter_defaults = { .max_ammo = 42, .scale = 2 };
static const snake_params_t  snake_defaults  = { .scale = 4 };
static const pwm_test_params_t pwm_test_defaults = { .pad = 0 };
static const nfc_test_params_t nfc_test_defaults = { .pad = 0 };

static app_descriptor_t s_app_registery[] = 
{
    APP_ENTRY("pwm", pwm_test),
    APP_ENTRY("nfc", nfc_test),
    APP_ENTRY("ammo", ammo_counter),
    APP_ENTRY("snake", snake)
};
static const uint8_t s_app_count = ARRAY_LENGTH(s_app_registery);

typedef struct app_select_context_t
{
    button_context_t incr_button;
    button_context_t decr_button;
    button_context_t next_button;
    button_context_t prev_button;
    button_context_t select_app_button;

    cursor_type_t cursor;
    int16_t cursor_app;
    int16_t cursor_param;

    uint8_t param_buf[64];
} app_select_context_t;

static inline void* move_pointer(const void* a_ptr, size_t a_move)
{
    return (void*)((const unsigned char*)a_ptr + a_move);
}

static inline int wrap(int a_val, int a_max) 
{
    return ((a_val % a_max) + a_max) % a_max;
}

static inline int wrapminmax(int a_val, int a_min, int a_max) 
{
    int range = a_max - a_min;
    return ((((a_val - a_min) % range) + range) % range) + a_min;
}

static inline const app_descriptor_t* GetAppCursor(app_select_context_t* a_app_select)
{
    return &s_app_registery[a_app_select->cursor_app];
}

static inline const app_param_descriptor_t* GetParamCursor(app_select_context_t* a_app_select)
{
    return &s_app_registery[a_app_select->cursor_app].descriptors[a_app_select->cursor_param];
}

static inline const uint16_t get_u16_param(const app_param_descriptor_t* a_desc, void* a_data)
{
    return *(uint16_t*)move_pointer(a_data, a_desc->byte_offset);
}

static inline const uint8_t get_u8_param(const app_param_descriptor_t* a_desc, void* a_data)
{
    return *(uint8_t*)move_pointer(a_data, a_desc->byte_offset);
}

static inline const bool get_bool_param(const app_param_descriptor_t* a_desc, void* a_data)
{
    return *(bool*)move_pointer(a_data, a_desc->byte_offset);
}

static inline void display_app_info(app_select_context_t* a_app_select, render_context_t* a_ctx)
{
    const app_descriptor_t* cursor = GetAppCursor(a_app_select);
    uint16_t select_color = COLOR_GREEN;
    if (a_app_select->cursor == CURSOR_TYPE_APP)
        select_color = COLOR_RED;
    render_8x16glyphs(a_ctx, cursor->name, cursor->name_len, 4, 3, select_color, COLOR_BLACK, 16, a_ctx->height / 4 * 1);
}

static inline void display_param_info(app_select_context_t* a_app_select, render_context_t* a_ctx)
{
    const app_descriptor_t* cursor = GetAppCursor(a_app_select);
    const app_param_descriptor_t* param = GetParamCursor(a_app_select);
    uint16_t param_color = COLOR_GREEN;
    uint16_t param_val_color = COLOR_GREEN;
    if (a_app_select->cursor == CURSOR_TYPE_PARAM)
        param_color = COLOR_RED;
    else if (a_app_select->cursor == CURSOR_TYPE_PARAM_VAL)
        param_val_color = COLOR_RED;

    render_8x16glyphs(a_ctx, param->label, param->label_len, 4, 3, param_color, COLOR_BLACK, 16, a_ctx->height / 4 * 2);
    switch (param->type)
    {
    case PARAM_U16:
    {
        const uint16_t val = get_u16_param(param, (void*)a_app_select->param_buf);
        char str[5] = {0, 0, 0, 0, 0};
        if (val)
        {
            str[0] = '0' + (val / 10000);
            str[1] = '0' + (val / 1000);
            str[2] = '0' + (val / 100);
            str[3] = '0' + (val / 10);
            str[4] = '0' + (val % 10);
        }
        render_8x16glyphs(a_ctx, str, 5, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 4 * 3);
        break;
    }
    case PARAM_U8:
    {
        const uint8_t val = get_u8_param(param, (void*)a_app_select->param_buf);
        char str[3] = { 0, 0, 0 };
        if (val)
        {
            str[0] = '0' + (val / 100);
            str[1] = '0' + (val / 10);
            str[2] = '0' + (val % 10);
        }
        render_8x16glyphs(a_ctx, str, 3, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 4 * 3);
        break;
    }
    case PARAM_BOOL:
    {
        if (get_bool_param(param, (void*)a_app_select->param_buf))
            render_8x16glyphs(a_ctx, "true", 4, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 4 * 3);
        else
            render_8x16glyphs(a_ctx, "false", 5, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 4 * 3);
        break;
    }
    }
}

static inline void modify_param(void* a_data, param_type_t a_type, int a_incr, int a_min, int a_max)
{
    switch (a_type)
    {
    case PARAM_U16:
        (*(uint16_t*)a_data) = (uint16_t)wrapminmax((*(uint16_t*)a_data) += a_incr, a_min, a_max);
        break;
    case PARAM_U8:
        (*(uint8_t*)a_data) = (uint8_t)wrapminmax((*(uint8_t*)a_data) += a_incr, a_min, a_max);
        break;
    case PARAM_BOOL:
        (*(bool*)a_data) = !(*(bool*)a_data);
        break;
    }
}

static void move_cursor(app_select_context_t* a_app_select, int a_incr)
{
    if (a_app_select->cursor == CURSOR_TYPE_APP)
    {
        int new_pos = a_app_select->cursor_app + a_incr;
        a_app_select->cursor_app = wrap(a_app_select->cursor_app + a_incr, s_app_count);
        const app_descriptor_t* cursor = GetAppCursor(a_app_select);
        memcpy(a_app_select->param_buf, cursor->params, cursor->params_size);
    }
    else if (a_app_select->cursor == CURSOR_TYPE_PARAM)
    {
        const app_descriptor_t* app = GetAppCursor(a_app_select);
        a_app_select->cursor_param = wrap(a_app_select->cursor_param + a_incr, app->descriptor_count);
    }
    else if (a_app_select->cursor == CURSOR_TYPE_PARAM_VAL)
    {
        const app_descriptor_t* app = GetAppCursor(a_app_select);
        const app_param_descriptor_t* param = GetParamCursor(a_app_select);
        modify_param(&a_app_select->param_buf[param->byte_offset], param->type, a_incr, param->min, param->max);
    }
}

static void change_app(app_select_context_t* a_app_select, app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{
    int16_t cursor = a_app_select->cursor_app;
    uint8_t param_buf[64];
    memcpy(param_buf, a_app_select->param_buf, sizeof(param_buf));
    a_app->close(a_app);

    memory_arena_set_marker(a_arena, a_app->memory_arena_marker);
    memory_set(a_app, 0, sizeof(a_app));
    
    s_app_registery[cursor].init(a_app, a_arena, a_ctx, param_buf);
}

void app_select_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(app_select_context_t));
    a_app->update = app_select_update;
    a_app->render = app_select_render;
    a_app->close = app_select_close;
    
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    app_select->cursor = CURSOR_TYPE_APP;
    app_select->cursor_app = 0;
    app_select->cursor_param = 0;
    button_init_context(&app_select->incr_button, 1, 10);
    button_init_context(&app_select->decr_button, 2, 10);
    button_init_context(&app_select->next_button, 3, 10);
    button_init_context(&app_select->prev_button, 4, 10);
    button_init_context(&app_select->select_app_button, 9, 10);

    move_cursor(app_select, 0);
    app_select_render(a_app, a_ctx);
}

app_update_status_t app_select_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    button_update(&app_select->incr_button, a_now_ms);
    button_update(&app_select->decr_button, a_now_ms);
    button_update(&app_select->next_button, a_now_ms);
    button_update(&app_select->prev_button, a_now_ms);
    button_update(&app_select->select_app_button, a_now_ms);

    int incr_cursor = button_pressed(&app_select->incr_button) + -button_pressed(&app_select->decr_button);
    if (incr_cursor != 0)
    {
        move_cursor(app_select, incr_cursor);
        return APP_RENDER;
    }
    int switch_cursor = button_pressed(&app_select->next_button) + -button_pressed(&app_select->prev_button);
    if (switch_cursor != 0)
    {
        app_select->cursor = wrap(app_select->cursor + switch_cursor, CURSOR_TYPE_MAX);
        return APP_RENDER;
    }

    if (button_pressed(&app_select->select_app_button))
    {
        change_app(app_select, a_app, a_arena, a_ctx);
        return APP_EXIT_NO_CLOSE;
    }

    return APP_OK;
}

void app_select_render(app_context_t* a_app, render_context_t* a_ctx)
{
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    render_fill(a_ctx, COLOR_BLACK);
    display_app_info(app_select, a_ctx);
    display_param_info(app_select, a_ctx);
}

void app_select_close(app_context_t* a_app)
{
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    button_free_context(&app_select->incr_button);
    button_free_context(&app_select->decr_button);
    button_free_context(&app_select->next_button);
    button_free_context(&app_select->prev_button);
    button_free_context(&app_select->select_app_button);
}
