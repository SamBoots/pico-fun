#ifndef IO_TYPES_H
#define IO_TYPES_H

typedef struct button_context_t
{
    u32 last_change_ms;
    u16 debounce_ms;
    u8 pin;
    bool previous;
    bool current;
} button_context_t;

#endif // IO_TYPES_H
