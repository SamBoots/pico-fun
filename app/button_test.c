#include "pico/stdlib.h"
#include "app.h"
#include "../io/io_types.h"
#include "../io/button.h"
#include "../memory/memory_arena.h"
#include "../graphics/renderer.h"
#include "../graphics/render_types.h"
#include "math.h"

typedef enum { BUTTON_RELEASE, BUTTON_PRESSED, BUTTON_HELD } button_status_t;

#define BUTTON_MAX 8

typedef struct button_test_params_t
{
    uint8_t button_count;
    uint8_t button_pin[BUTTON_MAX];
} button_test_params_t;

typedef struct button_test_context_t
{
    button_context_t buttons[BUTTON_MAX];
    button_status_t button_old_statuses[BUTTON_MAX];
    button_status_t button_new_statuses[BUTTON_MAX];
    int16_t sector_width;
    int16_t sector_height;
    bool height_split;
    int8_t button_count;
} button_test_context_t;

static void button_status_string(button_status_t a_status, const char** a_pstring, uint16_t* a_pstr_len)
{
    switch (a_status)
    {
    case BUTTON_RELEASE:
        *a_pstring = "release";
        *a_pstr_len = 8;
        break;
    case BUTTON_PRESSED:
        *a_pstring = "pressed";
        *a_pstr_len = 7;
        break;
    case BUTTON_HELD:
        *a_pstring = "held";
        *a_pstr_len = 4;
        break;
    }
}

static void button_draw_areas(render_context_t* a_ctx, button_test_context_t* a_app_ctx, uint8_t a_button_count)
{
    if (a_app_ctx->height_split)
    {
        uint16_t ping_pong_color_0 = COLOR_BLUE;
        uint16_t ping_pong_color_1 = COLOR_GREEN;
        for (size_t i = 0; i < a_button_count; i++)
        {
            uint16_t width = (i >= 4) ? a_app_ctx->sector_width : 0;
            uint16_t height = (i % 4) * a_app_ctx->sector_height;
            if (a_app_ctx->button_new_statuses[i] != a_app_ctx->button_old_statuses[i])
            {
                if (ping_pong_color_0 == COLOR_GREEN)
                {
                    ping_pong_color_0 = COLOR_BLUE;
                    ping_pong_color_1 = COLOR_GREEN;
                }
                else
                {
                    ping_pong_color_0 = COLOR_GREEN;
                    ping_pong_color_1 = COLOR_BLUE;
                }
                const char* str;
                uint16_t str_len;
                button_status_string(a_app_ctx->button_new_statuses[i], &str, &str_len);
                render_draw_rect(a_ctx, width, height, width + a_app_ctx->sector_width, height + a_app_ctx->sector_height, ping_pong_color_1);
                render_8x16glyphs(a_ctx, str, str_len, 4, 2, ping_pong_color_0, ping_pong_color_1, width, height);
            }
        }
    }
    else
    {
        uint16_t ping_pong_color_0 = COLOR_BLUE;
        uint16_t ping_pong_color_1 = COLOR_GREEN;
        for (size_t i = 0; i < a_button_count; i++)
        {
            uint16_t width = (i % 4) * a_app_ctx->sector_width;
            uint16_t height = (i >= 4) ? a_app_ctx->sector_height : 0;
            if (a_app_ctx->button_new_statuses[i] != a_app_ctx->button_old_statuses[i])
            {
                if (ping_pong_color_0 == COLOR_GREEN)
                {
                    ping_pong_color_0 = COLOR_BLUE;
                    ping_pong_color_1 = COLOR_GREEN;
                }
                else
                {
                    ping_pong_color_0 = COLOR_GREEN;
                    ping_pong_color_1 = COLOR_BLUE;
                }
                const char* str;
                uint16_t str_len;
                button_status_string(a_app_ctx->button_new_statuses[i], &str, &str_len);
                render_draw_rect(a_ctx, width, height, width + a_app_ctx->sector_width, height + a_app_ctx->sector_height, ping_pong_color_1);
                render_8x16glyphs(a_ctx, str, str_len, 4, 2, ping_pong_color_0, ping_pong_color_1, width, height);
            }
        }
    }
}

static app_update_status_t button_test_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, uint32_t a_now_ms)
{
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
    
    app_update_status_t status = APP_OK;
    for (size_t i = 0; i < app_ctx->button_count; i++)
    {
        button_update(&app_ctx->buttons[i], a_now_ms);
        app_ctx->button_old_statuses[i] = app_ctx->button_new_statuses[i];
        if (button_pressed(&app_ctx->buttons[i]))
        {
            app_ctx->button_new_statuses[i] = BUTTON_PRESSED;
            status = APP_RENDER;
        }
        if (button_released(&app_ctx->buttons[i]))
        {
            app_ctx->button_new_statuses[i] = BUTTON_RELEASE;
            status = APP_RENDER;
        }
        if (button_held(&app_ctx->buttons[i]))
        {
            app_ctx->button_new_statuses[i] = BUTTON_HELD;
            status = APP_RENDER;
        }
    }

    // never render
    return status;
}

static void button_test_render(app_context_t* a_app, render_context_t* a_ctx)
{
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
    button_draw_areas(a_ctx, app_ctx, app_ctx->button_count);
}

static void button_test_close(app_context_t* a_app)
{
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
    for (size_t i = 0; i < app_ctx->button_count; i++)
        button_free_context(&app_ctx->buttons[i]);
}

static void button_test_default_sizes(size_t* a_param_buf_size, size_t* a_desc_count)
{
    *a_param_buf_size = sizeof(button_test_params_t);
    *a_desc_count = 8;
}

static void button_test_default_params(uint8_t* a_param_buf, app_param_descriptor_t* a_descs)
{
    button_test_params_t* defaults = (button_test_params_t*)a_param_buf;
    defaults->button_count = 1;
    defaults->button_pin[0] = 1;

    a_descs[0] = APP_PARAM("count", PARAM_U8, offsetof(button_test_params_t, button_count), 1, BUTTON_MAX);
    a_descs[1] = APP_PARAM("pin0", PARAM_U8, offsetof(button_test_params_t, button_pin[0]), 1, 26);
    a_descs[2] = APP_PARAM("pin1", PARAM_U8, offsetof(button_test_params_t, button_pin[1]), 1, 26);
    a_descs[3] = APP_PARAM("pin2", PARAM_U8, offsetof(button_test_params_t, button_pin[2]), 1, 26);
    a_descs[4] = APP_PARAM("pin3", PARAM_U8, offsetof(button_test_params_t, button_pin[3]), 1, 26);
    a_descs[5] = APP_PARAM("pin4", PARAM_U8, offsetof(button_test_params_t, button_pin[4]), 1, 26);
    a_descs[6] = APP_PARAM("pin5", PARAM_U8, offsetof(button_test_params_t, button_pin[5]), 1, 26);
    a_descs[7] = APP_PARAM("pin6", PARAM_U8, offsetof(button_test_params_t, button_pin[6]), 1, 26);
    a_descs[8] = APP_PARAM("pin7", PARAM_U8, offsetof(button_test_params_t, button_pin[7]), 1, 26);
}

static void button_test_init_app(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, const void* a_app_params)
{
    a_app->memory_arena_marker = memory_arena_get_marker(a_arena);
    a_app->user_data = memory_arena_allocate(a_arena, sizeof(button_test_context_t));
    a_app->update = button_test_update;
    a_app->render = button_test_render;
    a_app->close = button_test_close;
    
    const button_test_params_t* params = (const button_test_params_t*)a_app_params;
    button_test_context_t* app_ctx = (button_test_context_t*)a_app->user_data;
    app_ctx->button_count = params->button_count;
    
    app_ctx->height_split = a_ctx->height > a_ctx->width;
    if (app_ctx->height_split)
    {
        app_ctx->sector_width = a_ctx->width / 2;
        app_ctx->sector_height = a_ctx->height / 4;
    }
    else
    {
        app_ctx->sector_width = a_ctx->width / 4;
        app_ctx->sector_height = a_ctx->height / 2;
    }

    for (size_t i = 0; i < params->button_count; i++)
        button_init_context(&app_ctx->buttons[i], params->button_pin[i], 20);

    for (size_t i = 0; i < BUTTON_MAX; i++)
    {
        app_ctx->button_new_statuses[i] = BUTTON_RELEASE;
        app_ctx->button_old_statuses[i] = BUTTON_PRESSED;
    }

    button_draw_areas(a_ctx, app_ctx, BUTTON_MAX);
}

APP_REGISTER(button_test);
