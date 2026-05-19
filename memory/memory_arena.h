#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

#define MEMORY_ARENA_SIZE 1024 * 4 // 4kb

typedef struct memory_arena_t
{
    uint8_t buffer[MEMORY_ARENA_SIZE];
    uint32_t offset;
} memory_arena_t;

uint8_t* memory_arena_allocate(memory_arena_t* a_arena, uint32_t a_size);
uint32_t memory_arena_get_marker(memory_arena_t* a_arena);
void memory_arena_set_marker(memory_arena_t* a_arena, uint32_t a_marker);

void* memory_set(void* a_src, int a_val, uint32_t a_len);

#endif // MEMORY_ARENA_H
