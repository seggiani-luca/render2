#ifndef SCENE_H
#define SCENE_H

#include "entity/entity.h"

// -- scenes

// a scene is just an entity hierarchy
struct scene {
	// scene name
	char name[ENT_NAME_SIZ];

	// root of scene hierarchy
	entity root;

	// rendering state
	renderScene render;	
};
typedef struct scene scene;

// creates a new scene
scene* newScene(const char* name);

// creates a new scene with default entities 
scene* newDefaultScene(const char* name);

// frees a scene
void freeScene(scene* s);

// scene iterator
typedef struct {
	// current entity
	entity* cur;

	// current depth in scene hierarchy
	int depth;
} sceneIter;

// gets a scene iterator for a scene
sceneIter getScIter(scene* s);

// advances a scene iterator
entity* scIterNext(sceneIter* it);

#endif
