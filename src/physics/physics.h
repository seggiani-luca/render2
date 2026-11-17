#ifndef PHYSICS_H
#define PHYSICS_H

#include "../math/math.h"

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
	// entity transformation (TODO needed?)
	mat4 transform;

	// next physics entity in list
	struct physicsEntity* next;
};
typedef struct physicsEntity physicsEntity;

// a physics view of a scene
typedef struct {
	// root of scene hierarchy
	physicsEntity* root;
} physicsScene;

// updates physics view of a scene
void updatePhysicsScene(scene* scene);

// frees physics scene hierarchy
void freePhysicsScene(scene* scene);

// prints physics view of a scene
void printPhysicsScene(scene* scene);

#endif
