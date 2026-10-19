#include "types.h"
#include "memory_arena.h"
#include "string.h"

u8* memory_arena_allocate(memory_arena_t* a_arena, u32 a_size)
{
    u32 old_offset = a_arena->offset;
    u32 new_offset = old_offset + a_size;
    if (new_offset > sizeof(a_arena->buffer))
    {
        return NULL;
    }
    a_arena->offset = new_offset;
    memset(&a_arena->buffer[old_offset], 0, a_size);
    return &a_arena->buffer[old_offset];
}

u32 memory_arena_get_marker(memory_arena_t* a_arena)
{
    return a_arena->offset;
}

void memory_arena_set_marker(memory_arena_t* a_arena, u32 a_marker)
{
    a_arena->offset = a_marker;
}

void* memory_set(void* a_src, isize a_val, u32 a_len)
{
    return memset(a_src, a_val, a_len);
}
