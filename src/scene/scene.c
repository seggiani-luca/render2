#include "scene.h"
#include "entity/entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- scenes

scene* newScene(const char* name) {
	if(*name == '\0') return NULL;

	// allocate entity
	scene* s = malloc(sizeof(scene));
	if(!s) return NULL;

	// copy name and clear root
	strncpy(s->name, name, ENT_NAME_SIZ);
	s->name[ENT_NAME_SIZ - 1] = '\0';

	// setup root
	strcpy(s->root.name, "Root");
	s->root.root = NULL;
	s->root.fieldCount = 0;
	s->root.parent = s->root.child = s->root.peer = NULL;

	return s;
}

scene* newDefaultScene(const char* name) {
	scene* s = newScene(name);
	if(!s) return NULL;

	// default entities
	appendChild(&s->root, newCameraEntity("Camera"));
	appendChild(&s->root, newRenderableEntity("Entity"));

	return s;
}

void freeScene(scene* s) {
	freeEntityChildren(&s->root);
	free(s);
}

sceneIter getScIter(scene* s) {
	return (sceneIter){
		&s->root, 0
	};
}

entity* scIterNext(sceneIter* it) {
	if(!it->cur) return NULL;

	// first check children
	if(it->cur->child) {
		it->depth++;
		it->cur = it->cur->child;
		return it->cur;
	}

	// climb to first parent with peers
	while(it->cur && !it->cur->peer) {
		it->depth--;
		it->cur = it->cur->parent;
	}

	// return NULL when root reached
	if(!it->cur) return NULL;

	// go to peer
	it->cur = it->cur->peer;
	return it->cur;
}
