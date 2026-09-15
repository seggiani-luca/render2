#include "arena.h"
#include "../exception/exception.h"
#include <stddef.h>
#include <stdalign.h>

// steps for arena buffer increase
#define ARENA_SIZE_STEP 1024

// arena chunk
struct arenaChunk {
	// allocated data
    alignas(max_align_t) char data[ARENA_SIZE_STEP];

	// bump pointer
	size_t offset;

	// next pointer
	struct arenaChunk* next;
};

// allocates a new chunk
arenaChunk* newChunk() {
	arenaChunk* chunk = xmalloc(sizeof(arenaChunk));
	chunk->offset = 0;
	chunk->next = NULL;

	return chunk;
}

arena newArena() {
	arenaChunk* chunk = newChunk();

	return (arena){
		.first = chunk,
		.current = chunk
	};
}

void* arenaAlloc(arena* a, size_t size) {
	if(size > ARENA_SIZE_STEP) {
		logEvent(FATAL, MEMORY, "Arena allocation won't fit in chunk");
		dumpEvents();
		exit(1);
	}

	// align memory
	size_t alignment = alignof(max_align_t);
    size_t offset = (a->current->offset + alignment - 1) & ~(alignment - 1);

	// check if fits, otherwise extend 
	if(offset + size > ARENA_SIZE_STEP) {
		if(!a->current->next) a->current->next = newChunk();

		// use past ones
		a->current = a->current->next;
		a->current->offset = 0;
		offset = 0;
	}

	// bump
	a->current->offset = offset + size;

	return a->current->data + offset;
}

void resetArena(arena* a) {
	a->current = a->first;
	a->current->offset = 0;
}

void freeArena(arena* a) {
	if(!a) return;

	// free chunks
	arenaChunk* cur = a->first;
	while(cur) {
		arenaChunk* next = cur->next;
		free(cur);
		cur = next;
	}
}
