#include "stdint.h"
#include "memory_arena.h"
#include "string.h"

uint8_t* memory_arena_allocate(memory_arena_t* a_arena, uint32_t a_size)
{
    uint32_t old_offset = a_arena->offset;
    uint32_t new_offset = old_offset + a_size;
    if (new_offset > sizeof(a_arena->buffer))
    {
        return NULL;
    }
    a_arena->offset = new_offset;
    memset(&a_arena->buffer[old_offset], 0, a_size);
    return &a_arena->buffer[old_offset];
}

uint32_t memory_arena_get_marker(memory_arena_t* a_arena)
{
    return a_arena->offset;
}

void memory_arena_set_marker(memory_arena_t* a_arena, uint32_t a_marker)
{
    a_arena->offset = a_marker;
}

void* memory_set(void* a_src, int a_val, uint32_t a_len)
{
    return memset(a_src, a_val, a_len);
}
