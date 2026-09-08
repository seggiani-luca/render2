#include "arena.h"

#include <stddef.h>
#include <stdalign.h>

arena newArena(size_t capacity) {
	void* data = malloc(capacity);
	if(!data) capacity = 0;

	return (arena){
		.data = data,
		.capacity = capacity,
		.offset = 0
	};
}

void* arenaAlloc(arena* a, size_t size) {
	// align memory
	size_t alignment = alignof(max_align_t);
    size_t offset = (a->offset + alignment - 1) & ~(alignment - 1);

	if(offset + size > a->capacity) return NULL;

	// bump
	a->offset = offset + size;

	return a->data + offset;
}

void resetArena(arena* a) {
	a->offset = 0;
}

void freeArena(arena* a) {
	free(a->data);
	a->capacity = 0;
}
