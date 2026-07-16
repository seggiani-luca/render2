#include "entity.h"
#include "../../gui/inspector/inspector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- fields

// helper for field creation
field newField(const char* name, fieldVtable* vtable) {
	// create field on stack
	field f;

	// init name and vtable
	strncpy(f.name, name, ENT_NAME_SIZ);
	f.name[ENT_NAME_SIZ - 1] = '\0';
	f.vtable = vtable;

	// copy back
	return f;
}

void readField(const field* f, void* dst) { f->vtable->read(f, dst); }
void writeField(field* f, const void* src) { f->vtable->write(f, src); }
void printField(const field* f) { f->vtable->print(f); }
int guiField(const field* f, guiContext* ctx) { return f->vtable->gui(f, ctx);}

// macro for vtable declaration
#define VTABLE(type)                  \
	fieldVtable type##FieldVtable = { \
	    .read  = type##Read,          \
	    .write = type##Write,         \
	    .print = type##FieldPrint,    \
	    .gui   = type##FieldGui,      \
	};

// macro for field allocation
#define ALLOC_FIELD(type)                             \
	if(*name == '\0') return NULL;                    \
	type##Field* f = malloc(sizeof(type##Field)); \
	if(!f) return NULL;                               \
	f->base = newField(name, &type##FieldVtable);

// -- integer field

// reads an integer field
void intRead(const field* f, void* dst) {
	*(int*)dst = ((intField*)f)->val;
}

// writes an integer field
void intWrite(field* f, const void* src) {
	((intField*)f)->val = *(int*)src;
}

// debug prints an integer field
void intFieldPrint(const field* f) {
	printf("%s (Integer): %d", f->name, ((intField*)f)->val);
}

VTABLE(int)

field* intNew(const char* name) {
	ALLOC_FIELD(int)
	f->val = 0;

	return (field*)f;
}

// -- float field

// reads a float field
void floatRead(const field* f, void* dst) {
	*(float*)dst = ((floatField*)f)->val;
}

// writes a float field
void floatWrite(field* f, const void* src) {
	((floatField*)f)->val = *(float*)src;
}

// debug prints a float field
void floatFieldPrint(const field* f) {
	printf("%s (Float): %f", f->name, ((floatField*)f)->val);
}

VTABLE(float)

field* floatNew(const char* name) {
	ALLOC_FIELD(float)
	f->val = 0.0f;

	return (field*)f;
}

// -- string field

// reads a string field
void stringRead(const field* f, void* dst) {
	*(char**)dst = ((stringField*)f)->str;
}

// writes a string field
void stringWrite(field* f, const void* src) {
	stringField* sf = (stringField*)f;
	strncpy(sf->str, *(char**)src, ENT_STR_SIZ);
	sf->str[ENT_STR_SIZ - 1] = '\0';
}

// debug prints a string field
void stringFieldPrint(const field* f) {
	printf("%s (String): %s", f->name, ((stringField*)f)->str);
}

VTABLE(string)

field* stringNew(const char* name) {
	ALLOC_FIELD(string)
	*f->str = '\0';

	return (field*)f;
}

// -- vector fields

#define VEC_FIELD_IMPL(n)                             \
	void float##n##Read(const field* f, void* dst) {  \
	    *(float##n*)dst = ((float##n##Field*)f)->val; \
	}                                                 \
	                                                  \
	void float##n##Write(field* f, const void* src) { \
	    ((float##n##Field*)f)->val = *(float##n*)src; \
	}                                                 \
	                                                  \
	void float##n##FieldPrint(const field* f) {       \
	    float##n* vec = &((float##n##Field*)f)->val;  \
	    printf("%s (Vector): ", f->name);             \
		vecPrint##n(*vec);                            \
	}                                                 \
	                                                  \
	VTABLE(float##n)                                  \
	                                                  \
	field* float##n##New(const char* name) {          \
	    ALLOC_FIELD(float##n)                         \
	    f->val = (float##n){0};                       \
	                                                  \
	    return (field*)f;                             \
	}

// 2D vector field
VEC_FIELD_IMPL(2)

// 3D vector field
VEC_FIELD_IMPL(3)

// 4D vector field
VEC_FIELD_IMPL(4)

// -- matrix fields

#define MAT_FIELD_IMPL(n)                             \
	void mat##n##Read(const field* f, void* dst) {    \
	    *(mat##n*)dst = ((mat##n##Field*)f)->val;     \
	}                                                 \
	                                                  \
	void mat##n##Write(field* f, const void* src) {   \
	    ((mat##n##Field*)f)->val = *(mat##n*)src;     \
	}                                                 \
	                                                  \
	void mat##n##FieldPrint(const field* f) {         \
	    mat##n* mat = &((mat##n##Field*)f)->val;      \
	    printf("%s (Matrix): ", f->name);             \
		matPrint##n(*mat);                            \
	}                                                 \
	                                                  \
	VTABLE(mat##n)                                    \
	                                                  \
	field* mat##n##New(const char* name) {            \
	    ALLOC_FIELD(mat##n)                           \
	    f->val = (mat##n){0};                         \
	                                                  \
	    return (field*)f;                             \
	}

// 2x2 matrix field
MAT_FIELD_IMPL(2)

// 3x3 matrix field
MAT_FIELD_IMPL(3)

// 4x4 matrix field
MAT_FIELD_IMPL(4)

// -- quaternion field

// reads a quaternion field
void quatRead(const field* f, void* dst) {
	*(quat*)dst = ((quatField*)f)->val;
}

// writes a quaternion field
void quatWrite(field* f, const void* src) {
	((quatField*)f)->val = *(quat*)src;
}

// debug prquats a quaternion field
void quatFieldPrint(const field* f) {
	printf("%s (Quaternion): ", f->name);
	quatPrint(((quatField*)f)->val);
}

VTABLE(quat)

field* quatNew(const char* name) {
	ALLOC_FIELD(quat)
	f->val = (quat){0};

	return (field*)f;
}

// -- transform field

// reads a transform field
void transformRead(const field* f, void* dst) {
	*(transform*)dst = ((transformField*)f)->val;
}

// writes a transform field
void transformWrite(field* f, const void* src) {
	((transformField*)f)->val = *(transform*)src;
}

// debug prints a transform field
void transformFieldPrint(const field* f) {
	transform* t = &((transformField*)f)->val;
	float3 euler = quatToEuler(t->rotation);
	printf("%s (Transform): Position: %f, %f, %f, Rotation: %f, %f, %f, Scale: %f, %f, %f",
		f->name,
		t->position.x, t->position.y, t->position.z,
		euler.x, euler.y, euler.z,
		t->scale.x, t->scale.y, t->scale.z);
}

VTABLE(transform)

field* transformNew(const char* name) {
	ALLOC_FIELD(transform)
	f->val = (transform){0};
	f->val.scale = (float3){1.0f, 1.0f, 1.0f};

	return (field*)f;
}

// -- entities

void printEntity(const entity* e) {
	printf("%s (Entity):\n", e->name);
	field* cur = e->root;
	while(cur) {
		printf("\t");
		printField(cur);
		printf("\n");

		cur = cur->next;
	}
}

// -- lifetime

entity* newEntity(const char* name) {
	if(*name == '\0') return NULL;

	// allocate entity
	entity* e = malloc(sizeof(entity));
	if(!e) return NULL;

	// copy name
	strncpy(e->name, name, ENT_NAME_SIZ);
	e->name[ENT_NAME_SIZ - 1] = '\0';

	// setup fields
	e->root = NULL;
	e->fieldCount = 0;

	// setup hierarchy
	e->parent = e->child = e->peer = NULL;
	e->childCount = 0;

	return e;
}

void freeEntityChildren(entity* e) {
	// get child
	entity* child = e->child;

	// go through children, clearing
	while(child) {
		entity* next = child->peer;
		freeEntity(child);
		child = next;
	}
}

void freeEntity(entity* e) {
	// free children
	freeEntityChildren(e);

	// go through all fields, freeing
	field* cur = e->root;
	while(cur) {
		field* tmp = cur;
		cur = cur->next;
		free(tmp);
	}

	free(e);
}

// -- fields

void appendField(entity* e, void* f) {
	if(!f) return;

	// fre yourself if not valid
	if(getField(e, ((field*)f)->name)) {
		free(f);
		return;
	}

	// get to last field
	field** cur = &e->root;
	while(*cur) cur = &(*cur)->next;

	// append
	((field*)f)->next = NULL;
	*cur = f;
	e->fieldCount++;
}

void removeField(entity* e, const char* name) {
	// go through all fields
	field** cur = &e->root;
	while(*cur) {
		// remove by name
		if(strcmp((*cur)->name, name) == 0) {
			field* tmp = *cur;
			*cur = (*cur)->next;
			free(tmp);
			e->fieldCount--;

			return;
		}

		cur = &(*cur)->next;
	}
}

field* getField(const entity* e, const char* name) {
	// go through all fields
	field* cur = e->root;
	while(cur) {
		// return by name
		if(strcmp(cur->name, name) == 0) return cur;

		cur = cur->next;
	}

	return NULL;
}

// -- hierarchy

// helper for child count propagation
void updateChildCount(entity* e, int delta) {
	while(e) {
		e->childCount += delta;
		e = e->parent;
	}
}

// appends a child to an entity
void appendChild(entity* e, entity* child) {
	if(!child) return;

	child->parent = e;
	child->peer = NULL;

	// update count
	updateChildCount(e, 1 + child->childCount);

	// simple on first child
	if(!e->child) {
		e->child = child;

		return;
	}

	// walk all children
	entity* temp = e->child;
	while(temp->peer) temp = temp->peer;

	// append child
	temp->peer = child;
}

// removes a child from an entity
void removeChild(entity* e, entity* child) {
	if(!child) return;

	// simple on first child
	if(e->child == child) {
		e->child = child->peer;
		child->parent = NULL;
		child->peer = NULL;

		// update count
		updateChildCount(e, -1 - child->childCount);
		return;
	}

	// walk all children
	entity* temp = e->child;
	while(temp->peer && temp->peer != child) temp = temp->peer;

	// remove only if found
	if(!temp || temp->peer != child) return;
	temp->peer = child->peer;
	child->parent = NULL;
	child->peer = NULL;

	// update count
	updateChildCount(e, -1 - child->childCount);
}

void moveChild(entity* e, entity* child) {
	if(e == child) return;

	removeChild(child->parent, child);
	appendChild(e, child);
}
