#ifndef BUTTON_H
#define BUTTON_H

typedef struct button_context_t button_context_t;

bool button_init_context(button_context_t* a_ctx, u8 a_pin, u16 a_debounce_ms);
bool button_free_context(button_context_t* a_ctx);
bool button_update(button_context_t* a_ctx, u32 a_now_ms);

bool button_pressed(const button_context_t* a_ctx);
bool button_released(const button_context_t* a_ctx);
bool button_held(const button_context_t* a_ctx);
bool button_unheld(const button_context_t* a_ctx);

#endif // BUTTON_H
