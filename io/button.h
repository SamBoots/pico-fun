#ifndef BUTTON_H
#define BUTTON_H

typedef struct button_context_t button_context_t;

int button_init_context(button_context_t* a_ctx, uint16_t a_pin, uint16_t a_debounce_ms);
int button_free_context(button_context_t* a_ctx);
int button_update(button_context_t* a_ctx, uint32_t a_now_ms);

bool button_pressed(const button_context_t* a_ctx);
bool button_released(const button_context_t* a_ctx);
bool button_held(const button_context_t* a_ctx);

#endif // BUTTON_H
