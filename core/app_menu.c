#include "core/types.h"
#include "app/app.h"
#include "app_menu.h"
#include "io/button.h"
#include "io/fs.h"
#include "memory_arena.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"
#include "math.h"
#include "string.h"

typedef enum { CURSOR_TYPE_APP = 0, CURSOR_TYPE_PARAM = 1, CURSOR_TYPE_PARAM_VAL = 2, CURSOR_TYPE_SAVE_DEFAULT = 3, CURSOR_TYPE_RELOAD_DEFAULT = 4, CURSOR_TYPE_MAX = 5 } cursor_type_t;

#define PARAM_BUF_SIZE 64
#define PARAM_MAX_DESCRIPTORS 16
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

const app_entry_t* app_entry_begin(void) { return &__start_app_entries; }
const app_entry_t* app_entry_end(void)   { return &__stop_app_entries; }

typedef struct app_menu_context_t
{
    u32 memory_marker;
    
    button_context_t incr_button;
    button_context_t next_button;
    button_context_t select_app_button;
    button_context_t select_app_button_extra;

    cursor_type_t cursor;
    i16 cursor_app;
    i16 cursor_param;

    u16 app_count;

    u8 param_buf[PARAM_BUF_SIZE];
    size_t param_buf_size;
    app_param_descriptor_t app_descs[PARAM_MAX_DESCRIPTORS];
    size_t app_desc_count;
} app_menu_context_t;
static app_menu_context_t* g_menu;

static inline void* move_pointer(const void* a_ptr, size_t a_move)
{
    return (void*)((const unsigned char*)a_ptr + a_move);
}

static inline isize wrap(isize a_val, isize a_max) 
{
    return ((a_val % a_max) + a_max) % a_max;
}

static inline isize wrapminmax(isize a_val, isize a_min, isize a_max) 
{
    isize range = a_max - a_min;
    return ((((a_val - a_min) % range) + range) % range) + a_min;
}

static inline const app_entry_t* GetAppCursor(void)
{
    return &app_entry_begin()[g_menu->cursor_app];
}

static inline const app_param_descriptor_t* GetParamCursor(void)
{
    return &g_menu->app_descs[g_menu->cursor_param];
}

static inline const u16 get_u16_param(const app_param_descriptor_t* a_desc, void* a_data)
{
    return *(u16*)move_pointer(a_data, a_desc->byte_offset);
}

static inline const u8 get_u8_param(const app_param_descriptor_t* a_desc, void* a_data)
{
    return *(u8*)move_pointer(a_data, a_desc->byte_offset);
}

static inline const bool get_bool_param(const app_param_descriptor_t* a_desc, void* a_data)
{
    return *(bool*)move_pointer(a_data, a_desc->byte_offset);
}

static inline void display_app_info(render_context_t* a_ctx)
{
    const app_entry_t* cursor = GetAppCursor();
    u16 select_color = COLOR_GREEN;
    if (g_menu->cursor == CURSOR_TYPE_APP)
        select_color = COLOR_RED;
    render_8x16glyphs(a_ctx, cursor->name, cursor->name_len, 4, 3, select_color, COLOR_BLACK, 16, 0);
}

static inline void display_param_info(render_context_t* a_ctx)
{
    const app_param_descriptor_t* param = GetParamCursor();
    u16 param_color = COLOR_GREEN;
    u16 param_val_color = COLOR_GREEN;
    if (g_menu->cursor == CURSOR_TYPE_PARAM)
        param_color = COLOR_RED;
    else if (g_menu->cursor == CURSOR_TYPE_PARAM_VAL)
        param_val_color = COLOR_RED;

    render_8x16glyphs(a_ctx, param->label, param->label_len, 4, 3, param_color, COLOR_BLACK, 16, a_ctx->height / 6 * 1);
    switch (param->type)
    {
    case PARAM_U16:
    {
        const u16 val = get_u16_param(param, (void*)g_menu->param_buf);
        char str[5] = {0, 0, 0, 0, 0};
        if (val)
        {
            str[0] = '0' + (val / 10000);
            str[1] = '0' + (val / 1000);
            str[2] = '0' + (val / 100);
            str[3] = '0' + (val / 10);
            str[4] = '0' + (val % 10);
        }
        render_8x16glyphs(a_ctx, str, 5, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 2);
        break;
    }
    case PARAM_U8:
    {
        const u8 val = get_u8_param(param, (void*)g_menu->param_buf);
        char str[3] = { 0, 0, 0 };
        if (val)
        {
            str[0] = '0' + (val / 100);
            str[1] = '0' + (val / 10);
            str[2] = '0' + (val % 10);
        }
        render_8x16glyphs(a_ctx, str, 3, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 2);
        break;
    }
    case PARAM_BOOL:
    {
        if (get_bool_param(param, (void*)g_menu->param_buf))
            render_8x16glyphs(a_ctx, "true", 4, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 2);
        else
            render_8x16glyphs(a_ctx, "false", 5, 4, 3, param_val_color, COLOR_BLACK, 16, a_ctx->height / 6 * 2);
        break;
    }
    }
}

static inline void display_param_save_info(render_context_t* a_ctx)
{
    u16 save_color = COLOR_GREEN;
    u16 reload_color = COLOR_GREEN;
    if (g_menu->cursor == CURSOR_TYPE_SAVE_DEFAULT)
        save_color = COLOR_RED;
    else if (g_menu->cursor == CURSOR_TYPE_RELOAD_DEFAULT)
        reload_color = COLOR_RED;

    render_8x16glyphs(a_ctx, "save", 4, 4, 3, save_color, COLOR_BLACK, 16, a_ctx->height / 6 * 3);
    render_8x16glyphs(a_ctx, "reload", 6, 4, 3, reload_color, COLOR_BLACK, 16, a_ctx->height / 6 * 4);
}

static inline void modify_param(void* a_data, param_type_t a_type, isize a_incr, isize a_min, isize a_max)
{
    switch (a_type)
    {
    case PARAM_U16:
        (*(u16*)a_data) = (u16)wrapminmax((*(u16*)a_data) += a_incr, a_min, a_max);
        break;
    case PARAM_U8:
        (*(u8*)a_data) = (u8)wrapminmax((*(u8*)a_data) += a_incr, a_min, a_max);
        break;
    case PARAM_BOOL:
        (*(bool*)a_data) = !(*(bool*)a_data);
        break;
    }
}

static inline void load_app_params(void)
{
    const app_entry_t* cursor = GetAppCursor();
    cursor->default_sizes(&g_menu->param_buf_size, &g_menu->app_desc_count);

    // TODO, ERROR CHECK SIZES HERE

    cursor->default_params(g_menu->param_buf, g_menu->app_descs);

    // override default params, maybe make it a different function call?
    fs_read(cursor->name, cursor->name_len, g_menu->param_buf, g_menu->param_buf_size);
}

static void move_cursor(isize a_incr)
{
    if (g_menu->cursor == CURSOR_TYPE_APP)
    {
        isize new_pos = g_menu->cursor_app + a_incr;
        g_menu->cursor_app = wrap(g_menu->cursor_app + a_incr, g_menu->app_count);
        g_menu->cursor_param = 0;
        load_app_params();
    }
    else if (g_menu->cursor == CURSOR_TYPE_PARAM)
    {
        g_menu->cursor_param = wrap(g_menu->cursor_param + a_incr, g_menu->app_desc_count);
    }
    else if (g_menu->cursor == CURSOR_TYPE_PARAM_VAL)
    {
        const app_param_descriptor_t* param = GetParamCursor();
        modify_param(&g_menu->param_buf[param->byte_offset], param->type, a_incr, param->min, param->max);
    }
    else if (g_menu->cursor == CURSOR_TYPE_SAVE_DEFAULT)
    {
        const app_entry_t* app = GetAppCursor();
        fs_write(app->name, app->name_len, g_menu->param_buf, PARAM_BUF_SIZE);
    }
    else if (g_menu->cursor == CURSOR_TYPE_RELOAD_DEFAULT)
    {
        GetAppCursor()->default_params(g_menu->param_buf, g_menu->app_descs);
    }
}

static void change_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{
    i16 cursor = g_menu->cursor_app;
    u8 param_buf[64];
    memcpy(param_buf, g_menu->param_buf, sizeof(param_buf));
    a_app->close(a_app);

    memory_arena_set_marker(a_arena, a_app->memory_arena_marker);
    memory_set(a_app, 0, sizeof(a_app));
    
    GetAppCursor()->init(a_app, a_arena, a_ctx, param_buf);
}

void app_menu_init(memory_arena_t* a_arena, render_context_t* a_ctx)
{
    u32 marker = memory_arena_get_marker(a_arena);
    g_menu = (app_menu_context_t*)memory_arena_allocate(a_arena, sizeof(app_menu_context_t));
    g_menu->memory_marker = marker;
    
    g_menu->cursor = CURSOR_TYPE_APP;
    g_menu->cursor_app = 0;
    g_menu->cursor_param = 0;
    button_init_context(&g_menu->incr_button, 3, 10);
    button_init_context(&g_menu->next_button, 2, 10);
    button_init_context(&g_menu->select_app_button, 1, 10);
    button_init_context(&g_menu->select_app_button_extra, 14, 10);

    g_menu->app_count = app_entry_end() - app_entry_begin();
    move_cursor(0);
    app_menu_render(a_ctx);
}

app_init_t app_menu_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, u32 a_now_ms)
{
    button_update(&g_menu->incr_button, a_now_ms);
    button_update(&g_menu->next_button, a_now_ms);
    button_update(&g_menu->select_app_button, a_now_ms);
    button_update(&g_menu->select_app_button_extra, a_now_ms);

    isize incr_cursor = button_pressed(&g_menu->incr_button);
    if (incr_cursor != 0)
    {
        move_cursor(incr_cursor);
        return APP_RENDER;
    }
    isize switch_cursor = button_pressed(&g_menu->next_button);
    if (switch_cursor != 0)
    {
        g_menu->cursor = wrap(g_menu->cursor + switch_cursor, CURSOR_TYPE_MAX);
        return APP_RENDER;
    }

    if (button_pressed(&g_menu->select_app_button) || button_pressed(&g_menu->select_app_button_extra))
    {
        change_app(a_app, a_arena, a_ctx);
        return APP_EXIT;
    }

    return APP_OK;
}

void app_menu_render(render_context_t* a_ctx)
{
    render_fill(a_ctx, COLOR_BLACK);
    display_app_info(a_ctx);
    display_param_info(a_ctx);
    display_param_save_info(a_ctx);
}

void app_menu_close(void)
{
    button_free_context(&g_menu->incr_button);
    button_free_context(&g_menu->next_button);
    button_free_context(&g_menu->select_app_button);
    button_free_context(&g_menu->select_app_button_extra);
}
