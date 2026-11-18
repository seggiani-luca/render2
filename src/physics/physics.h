#ifndef PHYSICS_H
#define PHYSICS_H

#include "../math/math.h"
#include "../arena/arena.h"

// forward declaration of scene 
typedef struct scene scene;

// -- objects

// definition of rigidbody
// TODO implement

// definition of collider
// TODO implement

// -- views 

// an entity, which can be physically simulated
struct physicsEntity {
	// entity transformation
	mat4 transform;

	// next physics entity in list
	struct physicsEntity* next;
};
typedef struct physicsEntity physicsEntity;

// a physics view of a scene
typedef struct {
	// arena allocator
	arena a;
	
	// root of scene hierarchy
	physicsEntity* root;
} physicsScene;

// creates physics view of a scene
void newPhysicsScene(scene* scene);

// updates physics view of a scene
void updatePhysicsScene(scene* scene);

// rewinds physics view of a scene 
void rewindPhysicsScene(scene* scene);

// frees physics view of a scene 
void freePhysicsScene(scene* scene);

// prints physics view of a scene
void printPhysicsScene(scene* scene);

#endif
