#ifndef SCRIPT_H
#define SCRIPT_H

#include "lisp/lisp.h"

// forward declaration of field
typedef struct field field;

// forward declaration of scene and entity 
typedef struct scene scene;
typedef struct entity entity;

// -- constants

// start hook
#define START_HOOK "start"

// update hook
#define UPDATE_HOOK "update"

// -- natives

// logs values to console
value* nativeLog(arena* a, environment* env, value* args);

// throws an error 
value* nativeError(arena* a, environment* env, value* args);

// throws a warning
value* nativeWarn(arena* a, environment* env, value* args);

// -- execution

// starts a script in scene
void scriptStart(entity* ent);

// starts all scripts in scene
void scriptsStart(entity* ent);

// updates a script 
void scriptUpdate(entity* ent);

// updates all scripts in scene
void scriptsUpdate(entity* ent);

#endif
