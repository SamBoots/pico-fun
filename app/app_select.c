#include "pico/stdlib.h"
#include "app.h"
#include "app_select.h"
#include "ammo_counter.h"
#include "snake.h"
#include "pwm_test.h"
#include "nfc_test.h"
#include "button_test.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../io/fs.h"
#include "../memory/memory_arena.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "math.h"
#include "string.h"

typedef enum { CURSOR_TYPE_APP = 0, CURSOR_TYPE_PARAM = 1, CURSOR_TYPE_PARAM_VAL = 2, CURSOR_TYPE_SAVE_DEFAULT = 3, CURSOR_TYPE_RELOAD_DEFAULT = 4, CURSOR_TYPE_MAX = 5 } cursor_type_t;

typedef struct app_descriptor_t
{
    const char* name;
    uint16_t name_len;
    app_init_t init;
    void (*default_sizes)(size_t* a_param_buf_size, size_t* a_desc_count);
    void (*default_params)(uint8_t* a_param_buf, app_param_descriptor_t* a_descs);
} app_descriptor_t;

#define PARAM_BUF_SIZE 64
#define PARAM_MAX_DESCRIPTORS 16
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define APP_ENTRY(a_name, a_prefix)                    \
    {                                                  \
        .name             = a_name,                    \
        .name_len         = sizeof(a_name) - 1,        \
        .init             = a_prefix##_init_app,       \
        .default_sizes    = a_prefix##_default_sizes,  \
        .default_params   = a_prefix##_default_params  \
    }

static app_descriptor_t s_app_registery[] = 
{
    APP_ENTRY("ammo", ammo_counter),
    APP_ENTRY("snake", snake),
    APP_ENTRY("pwm", pwm_test),
    APP_ENTRY("nfc", nfc_test),
    APP_ENTRY("button", button_test)
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

    uint8_t param_buf[PARAM_BUF_SIZE];
    size_t param_buf_size;
    app_param_descriptor_t app_descs[PARAM_MAX_DESCRIPTORS];
    size_t app_desc_count;
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
    return &a_app_select->app_descs[a_app_select->cursor_param];
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
    render_8x16glyphs(a_ctx, cursor->name, cursor->name_len, 4, 3, select_color, COLOR_BLACK, 16, a_ctx->height / 6 * 1);
}

static inline void display_param_info(app_select_context_t* a_app_select, render_context_t* a_ctx)
{
    const app_param_descriptor_t* param = GetParamCursor(a_app_select);
    uint16_t param_color = COLOR_GREEN;
    uint16_t param_val_color = COLOR_GREEN;
    if (a_app_select->cursor == CURSOR_TYPE_PARAM)
        param_color = COLOR_RED;
    else if (a_app_select->cursor == CURSOR_TYPE_PARAM_VAL)
        param_val_color = COLOR_RED;

    render_8x16glyphs(a_ctx, param->label, param->label_len, 4, 3, param_color, COLOR_BLACK, 16, a_ctx->height / 6 * 2);
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
        render_8x16glyphs(a_ctx, str, 5, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 3);
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
        render_8x16glyphs(a_ctx, str, 3, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 3);
        break;
    }
    case PARAM_BOOL:
    {
        if (get_bool_param(param, (void*)a_app_select->param_buf))
            render_8x16glyphs(a_ctx, "true", 4, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 3);
        else
            render_8x16glyphs(a_ctx, "false", 5, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 3);
        break;
    }
    }
}

static inline void display_param_save_info(app_select_context_t* a_app_select, render_context_t* a_ctx)
{
    uint16_t save_color = COLOR_GREEN;
    uint16_t reload_color = COLOR_GREEN;
    if (a_app_select->cursor == CURSOR_TYPE_SAVE_DEFAULT)
        save_color = COLOR_RED;
    else if (a_app_select->cursor == CURSOR_TYPE_RELOAD_DEFAULT)
        reload_color = COLOR_RED;

    render_8x16glyphs(a_ctx, "save", 4, 4, 3, save_color, COLOR_BLACK, 16, a_ctx->height / 6 * 4);
    render_8x16glyphs(a_ctx, "reload", 6, 4, 3, reload_color, COLOR_BLACK, 16, a_ctx->height / 6 * 5);
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

static inline void load_app_params(app_select_context_t* a_app_select)
{
    s_app_registery[a_app_select->cursor_app].default_sizes(&a_app_select->param_buf_size, &a_app_select->app_desc_count);

    // TODO, ERROR CHECK SIZES HERE

    s_app_registery[a_app_select->cursor_app].default_params(a_app_select->param_buf, a_app_select->app_descs);

    const app_descriptor_t* cursor = GetAppCursor(a_app_select);
    // override default params, maybe make it a different function call?
    fs_read(cursor->name, cursor->name_len, a_app_select->param_buf, a_app_select->param_buf_size);
}

static void move_cursor(app_select_context_t* a_app_select, int a_incr)
{
    if (a_app_select->cursor == CURSOR_TYPE_APP)
    {
        int new_pos = a_app_select->cursor_app + a_incr;
        a_app_select->cursor_app = wrap(a_app_select->cursor_app + a_incr, s_app_count);
        a_app_select->cursor_param = 0;
        load_app_params(a_app_select);
    }
    else if (a_app_select->cursor == CURSOR_TYPE_PARAM)
    {
        a_app_select->cursor_param = wrap(a_app_select->cursor_param + a_incr, a_app_select->app_desc_count);
    }
    else if (a_app_select->cursor == CURSOR_TYPE_PARAM_VAL)
    {
        const app_param_descriptor_t* param = GetParamCursor(a_app_select);
        modify_param(&a_app_select->param_buf[param->byte_offset], param->type, a_incr, param->min, param->max);
    }
    else if (a_app_select->cursor == CURSOR_TYPE_SAVE_DEFAULT)
    {
        const app_descriptor_t* app = GetAppCursor(a_app_select);
        fs_write(app->name, app->name_len, a_app_select->param_buf, PARAM_BUF_SIZE);
    }
    else if (a_app_select->cursor == CURSOR_TYPE_RELOAD_DEFAULT)
    {
        const app_descriptor_t* app = GetAppCursor(a_app_select);
        s_app_registery[a_app_select->cursor_app].default_params(a_app_select->param_buf, a_app_select->app_descs);
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
    display_param_save_info(app_select, a_ctx);
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

void app_select_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count)
{
    a_param_buf_size = 0;
    a_desc_count = 0;
}

void app_select_default_params(uint8_t* a_param_buf, app_param_descriptor_t* a_descs)
{
    // dummy
    (void)a_param_buf;
    (void)a_descs;
}
