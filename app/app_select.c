#include "pico/stdlib.h"
#include "app.h"
#include "app_select.h"
#include "ammo_counter.h"
#include "snake.h"
#include "pwm_test.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"
#include "../graphics/render_types.h"
#include "../graphics/renderer.h"
#include "math.h"

typedef enum { PARAM_U16, PARAM_U8, PARAM_BOOL } param_type_t;

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
    void* params;
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

#define APP_ENTRY(a_name, a_prefix)                             \
    {                                                           \
        .name        = a_name,                                  \
        .name_len    = sizeof(a_name) - 1,                      \
        .init        = a_prefix##_init_app,                     \
        .params      = &a_prefix##_defaults,                    \
        .params_size = sizeof(a_prefix##_defaults),             \
        .descriptors      = a_prefix##_descriptors,             \
        .descriptor_count = ARRAY_LENGTH(a_prefix##_descriptors)\
    }

static const app_param_descriptor_t ammo_counter_descriptors[] = 
{
    APP_PARAM("max_ammo", PARAM_U8, offsetof(ammo_counter_params_t, max_ammo), 1, 99),
    APP_PARAM("scale", PARAM_U8, offsetof(ammo_counter_params_t, scale), 1, 16)
};

static const app_param_descriptor_t snake_descriptors[] = 
{
    APP_PARAM("scale", PARAM_U8, offsetof(snake_params_t, scale), 1, 16)
};

static const app_param_descriptor_t pwm_test_descriptors[] = 
{
    APP_PARAM("pad", PARAM_U8, offsetof(pwm_test_params_t, pad), 1, 16)
};

static const ammo_counter_params_t ammo_counter_defaults = { .max_ammo = 42, .scale = 2 };
static const snake_params_t  snake_defaults  = { .scale = 8 };
static const pwm_test_params_t pwm_test_defaults = { .pad = 0 };

static const app_descriptor_t s_app_registery[] = 
{
    APP_ENTRY("ammo", ammo_counter),
    APP_ENTRY("snake", snake),
    APP_ENTRY("pwm", pwm_test)
};
static const uint8_t s_app_count = ARRAY_LENGTH(s_app_registery);

typedef struct app_select_context_t
{
    button_context_t next_increase_button;
    button_context_t previous_decrease_button;
    button_context_t select_app_button;

    int16_t cursor_app;
} app_select_context_t;


static inline void display_app_info(app_select_context_t* a_app_select, render_context_t* a_ctx)
{
    app_descriptor_t* cursor = &s_app_registery[a_app_select->cursor_app];
    render_fill(a_ctx, COLOR_BLACK);
    render_8x16glyphs(a_ctx, cursor->name, cursor->name_len, 4, 3, COLOR_GREEN, COLOR_BLACK, 16, a_ctx->height / 2);
}

static void move_app_cursor(app_select_context_t* a_app_select, int a_incr)
{
    int new_pos = a_app_select->cursor_app + a_incr;
    if (new_pos == s_app_count)
    {
        new_pos = 0;
    }
    if (new_pos == -1)
    {
        new_pos = s_app_count - 1;
    }
    a_app_select->cursor_app = new_pos;
}

static void change_app(app_select_context_t* a_app_select, app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx)
{
    int16_t cursor = a_app_select->cursor_app;
    a_app->close(a_app);

    memory_arena_set_marker(a_arena, a_app->memory_arena_marker);
    memory_set(a_app, 0, sizeof(a_app));
    
    s_app_registery[cursor].init(a_app, a_arena, a_ctx, s_app_registery[cursor].params);
}

void app_select_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(app_select_context_t));
    a_app->update = app_select_update;
    a_app->render = app_select_render;
    a_app->close = app_select_close;
    
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    app_select->cursor_app = 0;
    button_init_context(&app_select->next_increase_button, 2, 10);
    button_init_context(&app_select->previous_decrease_button, 3, 10);
    button_init_context(&app_select->select_app_button, 9, 10);
    
    display_app_info(app_select, a_ctx);
}

app_update_status_t app_select_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    button_update(&app_select->next_increase_button, a_now_ms);
    button_update(&app_select->previous_decrease_button, a_now_ms);
    button_update(&app_select->select_app_button, a_now_ms);

    int incr = button_pressed(&app_select->next_increase_button) + -button_pressed(&app_select->previous_decrease_button);

    if (incr != 0)
    {
        move_app_cursor(app_select, incr);
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
    display_app_info(app_select, a_ctx);
}

void app_select_close(app_context_t* a_app)
{
    app_select_context_t* app_select = (app_select_context_t*)a_app->user_data;
    button_free_context(&app_select->next_increase_button);
    button_free_context(&app_select->previous_decrease_button);
    button_free_context(&app_select->select_app_button);
}
