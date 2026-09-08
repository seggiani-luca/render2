#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>

// -- arena allocator

// basic arena allocator
typedef struct {
	// allocated data
	void* data;

	// capacity of arena 
	size_t capacity;

	// bump pointer
	size_t offset;
} arena;

// allocates a new arena 
arena newArena(size_t capacity);

// allocates from the arena 
void* arenaAlloc(arena* a, size_t size);

// resets an arena
void resetArena(arena* a);

// frees an arena
void freeArena(arena* a);

#endif
