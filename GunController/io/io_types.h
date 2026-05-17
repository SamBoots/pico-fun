#ifndef IO_TYPES_H
#define IO_TYPES_H

typedef struct button_context_t
{
    uint16_t pin;
    bool previous;
    bool current;
    uint32_t last_change_ms;
} button_context_t;

#endif // IO_TYPES_H
