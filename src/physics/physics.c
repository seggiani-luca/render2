#include "physics.h"
#include "../scene/scene.h"
#include <stdlib.h>
#include <string.h>

// -- simulation 

// actually simulates a scene
void physics(scene* scn) {
	// update physics scene if modified
	if(scn->physicsDirty) updateRenderScene(scn);

}

// -- views

// max depth of scenes (TODO needed?)
#define SCN_MAX_DEPTH 16

// physics scene iterator
typedef struct {
	sceneIter base;

	// last depth 
	int lastDepth;

	// stack of transforms
	struct {
		// stack vector
		mat4 vec[SCN_MAX_DEPTH];

		// stack current pointer
		int cur;
	} stack;
} physicsSceneIter;

// gets a physics scene iterator for a scene
physicsSceneIter getPhysicsScIter(scene* s) {
	sceneIter base = getScIter(s);

	return (physicsSceneIter){
		.base = base,

		// reset stack
		.stack.vec[0] = transformToMat4(transformIdent()),

		// reset stack
		.lastDepth = 0,
		.stack.cur = 0	
	};
}

// advances a physics scene iterator
entity* physicsScIterNext(physicsSceneIter* it) {
	entity* new = scIterNext(&it->base);
	if(!new) return NULL;

	// compute delta
	int depthDelta = it->base.depth - it->lastDepth;

	// update stack cursor
	it->stack.cur += depthDelta;
	if (it->stack.cur < 0 || it->stack.cur >= SCN_MAX_DEPTH) return NULL;

	// get transform
	field* f = getField(new, REN_TRANSFORM_NAME);
	transform tf;
	if(f) tf = ((transformField*)f)->val;
	else tf = transformIdent();

	// convert to matrix 
	mat4 local = transformToMat4(tf);

	// compose with previous transform
	it->stack.vec[it->stack.cur] =
		matMul4(it->stack.vec[it->stack.cur - 1], local);

	it->lastDepth = it->base.depth;
	return new;
}

// gets current transform from physics scene iterator
mat4 physicsScTransform(physicsSceneIter* it) {
	return it->stack.vec[it->stack.cur];
}

// appens physics entity to physics scene
void appendPhysicsEntity(physicsScene* scn, physicsEntity* ent) {
	// append
	ent->next = scn->root;
	scn->root = ent;
}

void newPhysicsScene(scene* scene) {
	if(!scene) return;

	physicsScene* phs = &scene->physics;
	memset(phs, 0, sizeof(physicsScene));
	
	// create arena
	phs->a = newArena();
}

void updatePhysicsScene(scene* scene) {
	physicsScene* physics = &scene->physics;

	// rewind scene
	rewindPhysicsScene(scene);
	scene->physicsDirty = 0;

	// go through scene hierarchy
	physicsSceneIter iter = getPhysicsScIter(scene);
	do {
		entity* cur = iter.base.cur;
		mat4 curTrans = physicsScTransform(&iter);

		// get transform (field)
		field* transform = getField(cur, REN_TRANSFORM_NAME);

		// TODO get rigidbody and collider
	} while(physicsScIterNext(&iter));
}

void rewindPhysicsScene(scene* scene) {
	if(!scene) return;

	physicsScene* phs = &scene->physics;

	// free arena
	resetArena(&phs->a);

	// clear everything after arena
	memset((char*)phs + sizeof(phs->a), 0,
		sizeof(*phs) - sizeof(phs->a));
}

void freePhysicsScene(scene* scene) {
	if(!scene) return;

	physicsScene* phs = &scene->physics;

	// free arena
	freeArena(&phs->a);
}

void printPhysicsScene(scene* scene) {
	physicsScene* physics = &scene->physics;

	printf("%s (Physics View):\n", scene->name);

	// physics entities
	printf("Physics Entities:\n");
	physicsEntity* cur = physics->root;
	while(cur) {
		// TODO print rigidbody and collider

		printf("Transform:\n");
		matPrint4(cur->transform);

		printf("\n");
		cur = cur->next;
	}
		
	printf("\n");
}
