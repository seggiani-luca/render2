#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>

// -- arena allocator

typedef struct arenaChunk arenaChunk;

// basic arena allocator
typedef struct {
	// first arena chunk
	arenaChunk* first;

	// current arena chunk
	arenaChunk* current;
} arena;

// allocates a new arena 
arena newArena();

// allocates from the arena 
void* arenaAlloc(arena* a, size_t size);

// resets an arena
void resetArena(arena* a);

// frees an arena
void freeArena(arena* a);

#endif
