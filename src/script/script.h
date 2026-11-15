#ifndef SCRIPT_H
#define SCRIPT_H

#define NUM_CUSTOM_NATIVE_ENTRIES 38
#include "lisp/lisp.h"
#include "field/field.h"

// forward declaration of scene and entity 
typedef struct scene scene;
typedef struct entity entity;

// -- constants

// start hook
#define START_HOOK "start"

// update hook
#define UPDATE_HOOK "update"

// script hook keys
#define DELTA_TIME_KEY "delta-time"

// -- execution

// starts a script in scene
void scriptStart(entity* ent);

// starts all scripts in scene
void scriptsStart(entity* ent);

// updates a script 
void scriptUpdate(entity* ent, float deltaTime);

// updates all scripts in scene
void scriptsUpdate(entity* ent, float deltaTime);

#endif
