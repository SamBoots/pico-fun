#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

#define MEMORY_ARENA_SIZE 1024 * 4 // 4kb

typedef struct memory_arena_t
{
    u8 buffer[MEMORY_ARENA_SIZE];
    u32 offset;
} memory_arena_t;

u8* memory_arena_allocate(memory_arena_t* a_arena, u32 a_size);
u32 memory_arena_get_marker(memory_arena_t* a_arena);
void memory_arena_set_marker(memory_arena_t* a_arena, u32 a_marker);

void* memory_set(void* a_src, isize a_val, u32 a_len);

#endif // MEMORY_ARENA_H
