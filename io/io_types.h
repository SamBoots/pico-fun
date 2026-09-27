#ifndef IO_TYPES_H
#define IO_TYPES_H

typedef struct button_context_t
{
    uint32_t last_change_ms;
    uint16_t debounce_ms;
    uint8_t pin;
    bool previous;
    bool current;
} button_context_t;

#endif // IO_TYPES_H
