#ifndef APP_MENU_H
#define APP_MENU_H

// A special app, made it an app for easy

typedef struct memory_arena_t memory_arena_t;
typedef struct render_context_t render_context_t;

void app_menu_init(memory_arena_t* a_arena, render_context_t* a_ctx);
app_update_status_t app_menu_update(app_context_t* a_app, memory_arena_t* a_arena, render_context_t* a_ctx, u32 a_now_ms);
void app_menu_render(render_context_t* a_ctx);
void app_menu_close(void);


#endif // APP_MENU_H
