#ifndef SCRIPT_H
#define SCRIPT_H

#define NUM_CUSTOM_NATIVE_ENTRIES 13
#include "lisp/lisp.h"

// forward declaration of field
typedef struct field field;

// forward declaration of scene and entity 
typedef struct scene scene;
typedef struct entity entity;

// -- fields

// gets an integer field
value* intFieldGet(arena* a, const field* f);

// sets an integer field
void intFieldSet(field* f, const value* val);

// gets a float field
value* floatFieldGet(arena* a, const field* f);

// sets a float field
void floatFieldSet(field* f, const value* val);

// gets a string field
value* stringFieldGet(arena* a, const field* f);

// sets a string field
void stringFieldSet(field* f, const value* val);

// vector field get / set
#define VEC_VALUE_DECL(n)                                \
	value* float##n##FieldGet(arena* a, const field* f); \
	void float##n##FieldSet(field* f, const value* val);

// 2D vector field get / set
VEC_VALUE_DECL(2)

// 3D vector field get / set
VEC_VALUE_DECL(3)

// 4D vector field get / set
VEC_VALUE_DECL(4)

// matrix field get / set
#define MAT_VALUE_DECL(n)                              \
	value* mat##n##FieldGet(arena* a, const field* f); \
	void mat##n##FieldSet(field* f, const value* val);

// 2D matrix field get / set
MAT_VALUE_DECL(2)

// 3D matrix field get / set
MAT_VALUE_DECL(3)

// 4D matrix field get / set
MAT_VALUE_DECL(4)

// gets a quaternion field
value* quatFieldGet(arena* a, const field* f);

// sets a quaternion field
void quatFieldSet(field* f, const value* val);

// gets a transform field
value* transformFieldGet(arena* a, const field* f);

// sets a transform field
void transformFieldSet(field* f, const value* val);

// gets a camera field
value* cameraFieldGet(arena* a, const field* f);

// sets a camera field
void cameraFieldSet(field* f, const value* val);

// gets a atmosphere field
value* atmosphereFieldGet(arena* a, const field* f);

// sets a atmosphere field
void atmosphereFieldSet(field* f, const value* val);

// gets a texture field
value* textureFieldGet(arena* a, const field* f);

// sets a texture field
void textureFieldSet(field* f, const value* val);

// gets a mesh field
value* meshFieldGet(arena* a, const field* f);

// sets a mesh field
void meshFieldSet(field* f, const value* val);

// gets a material field
value* materialFieldGet(arena* a, const field* f);

// sets a material field
void materialFieldSet(field* f, const value* val);

// gets a script field
value* scriptFieldGet(arena* a, const field* f);

// sets a script field
void scriptFieldSet(field* f, const value* val);

// gets a shader field
value* shaderFieldGet(arena* a, const field* f);

// sets a shader field
void shaderFieldSet(field* f, const value* val);

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
