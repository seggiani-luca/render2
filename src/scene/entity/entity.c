#include "entity.h"
#include "../scene.h"
#include "../../gui/inspector/inspector.h"
#include "../../render/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- fields

// helper for field creation
field newField(const char* name, const fieldVtable* vtable) {
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
#define VTABLE(type)                               \
	static const fieldVtable type##FieldVtable = { \
	    .read  = type##FieldRead,                  \
	    .write = type##FieldWrite,                 \
	    .print = type##FieldPrint,                 \
	    .gui   = type##FieldGui,                   \
	};

// macro for vtable declaration, with free 
#define VTABLE_FREE(type)                          \
	static const fieldVtable type##FieldVtable = { \
	    .read  = type##FieldRead,                  \
	    .write = type##FieldWrite,                 \
	    .print = type##FieldPrint,                 \
	    .gui   = type##FieldGui,                   \
	    .free  = type##FieldFree                   \
	};

// macro for field allocation
#define ALLOC_FIELD(type)                         \
	if(*name == '\0') return NULL;                \
	type##Field* f = malloc(sizeof(type##Field)); \
	if(!f) return NULL;                           \
	f->base = newField(name, &type##FieldVtable);

// -- integer field

// reads an integer field
void intFieldRead(const field* f, void* dst) {
	*(int*)dst = ((intField*)f)->val;
}

// writes an integer field
void intFieldWrite(field* f, const void* src) {
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
void floatFieldRead(const field* f, void* dst) {
	*(float*)dst = ((floatField*)f)->val;
}

// writes a float field
void floatFieldWrite(field* f, const void* src) {
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
void stringFieldRead(const field* f, void* dst) {
	*(char**)dst = ((stringField*)f)->str;
}

// writes a string field
void stringFieldWrite(field* f, const void* src) {
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
	void float##n##FieldRead(const field* f, void* dst) {  \
	    *(float##n*)dst = ((float##n##Field*)f)->val; \
	}                                                 \
	                                                  \
	void float##n##FieldWrite(field* f, const void* src) { \
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
	void mat##n##FieldRead(const field* f, void* dst) {    \
	    *(mat##n*)dst = ((mat##n##Field*)f)->val;     \
	}                                                 \
	                                                  \
	void mat##n##FieldWrite(field* f, const void* src) {   \
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
void quatFieldRead(const field* f, void* dst) {
	*(quat*)dst = ((quatField*)f)->val;
}

// writes a quaternion field
void quatFieldWrite(field* f, const void* src) {
	((quatField*)f)->val = *(quat*)src;
}

// debug prints a quaternion field
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
void transformFieldRead(const field* f, void* dst) {
	*(transform*)dst = ((transformField*)f)->val;
}

// writes a transform field
void transformFieldWrite(field* f, const void* src) {
	((transformField*)f)->val = *(transform*)src;
}

// debug prints a transform field
void transformFieldPrint(const field* f) {
	transform t = ((transformField*)f)->val;
	printf("%s (Transform): ", f->name);
	transformPrint(t);
}

VTABLE(transform)

field* transformNew(const char* name) {
	ALLOC_FIELD(transform)
	f->val = transformIdent();

	return (field*)f;
}

// -- camera field

// reads a camera field
void cameraFieldRead(const field* f, void* dst) {
	*(camera*)dst = ((cameraField*)f)->val;
}

// writes a camera field
void cameraFieldWrite(field* f, const void* src) {
	((cameraField*)f)->val = *(camera*)src;
}

// debug prints a camera field
void cameraFieldPrint(const field* f) {
	camera* c = &((cameraField*)f)->val;
	printf("%s (Camera): Fov: %f, Near: %f, Far: %f",
		f->name, c->fov, c->nearPlane, c->farPlane);
}

VTABLE(camera)

field* cameraNew(const char* name) {
	ALLOC_FIELD(camera)
	f->val = (camera){0};
	f->val.fov = 90;
	f->val.nearPlane = 0.02;
	f->val.farPlane = 3000;

	return (field*)f;
}

// -- atmosphere field

// reads a camera field
void atmosphereFieldRead(const field* f, void* dst) {
	*(atmosphere*)dst = ((atmosphereField*)f)->val;
}

// writes a atmosphere field
void atmosphereFieldWrite(field* f, const void* src) {
	((atmosphereField*)f)->val = *(atmosphere*)src;
}

// debug prints an atmosphere field
void atmosphereFieldPrint(const field* f) {
	atmosphere* a = &((atmosphereField*)f)->val;
	printf("%s (Atmosphere): Ambient: %f, %f, %f",
		f->name, a->ambient.r, a->ambient.g, a->ambient.b);
}

VTABLE(atmosphere)

field* atmosphereNew(const char* name) {
	ALLOC_FIELD(atmosphere)
	f->val = (atmosphere){0};

	return (field*)f;
}

// -- texture field

// reads a texture field
void textureFieldRead(const field* f, void* dst) {
	*(dataRef**)dst = ((textureField*)f)->ref;
}

// writes a transform field
void textureFieldWrite(field* f, const void* src) {
	((textureField*)f)->ref = *(dataRef**)src;
}

void textureFieldPrint(const field* f) {
	dataRef* ref = ((textureField*)f)->ref;
	printf("%s (Texture): %s (%d refs)", f->name, ref->path, ref->refCount);
}

void textureFieldFree(field* f) {
	textureField* tf = (textureField*)f;
	if(tf->ref) textureFree(tf->ref->data);
}

VTABLE_FREE(texture);

field* textureNew(const char* name) {
	ALLOC_FIELD(texture)
	f->ref = NULL;

	return (field*)f;
}

// -- mesh field

// reads a mesh field
void meshFieldRead(const field* f, void* dst) {
	*(dataRef**)dst = ((meshField*)f)->ref;
}

// writes a transform field
void meshFieldWrite(field* f, const void* src) {
	((meshField*)f)->ref = *(dataRef**)src;
}

void meshFieldPrint(const field* f) {
	dataRef* ref = ((meshField*)f)->ref;
	printf("%s (Mesh): %s (%d refs)", f->name, ref->path, ref->refCount);
}

void meshFieldFree(field* f) {
	meshField* mf = (meshField*)f;
	if(mf->ref) meshFree(mf->ref->data);
}

VTABLE_FREE(mesh);

field* meshNew(const char* name) {
	ALLOC_FIELD(mesh)
	f->ref = NULL;

	return (field*)f;
}

// -- material field

// reads a material field
void materialFieldRead(const field* f, void* dst) {
	*(dataRef**)dst = ((materialField*)f)->ref;
}

// writes a transform field
void materialFieldWrite(field* f, const void* src) {
	((materialField*)f)->ref = *(dataRef**)src;
}

void materialFieldPrint(const field* f) {
	dataRef* ref = ((materialField*)f)->ref;
	printf("%s (Material): %s (%d refs)", f->name, ref->path, ref->refCount);
}

void materialFieldFree(field* f) {
	materialField* mf = (materialField*)f;
	if(mf->ref) materialFree(mf->ref->data);
}

VTABLE_FREE(material);

field* materialNew(const char* name) {
	ALLOC_FIELD(material)
	f->ref = NULL;

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
	if(strcmp(name, ROOT_NAME) == 0) return NULL;

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

entity* newRenderableEntity(const char* name) {
	entity* e = newEntity(name);
	if(!e) return NULL;

	// default fields
	appendField(e, transformNew(REN_TRANSFORM_NAME));
	appendField(e, meshNew(REN_MESH_NAME));
	appendField(e, materialNew(REN_MATERIAL_NAME));

	return e;
}

entity* newCameraEntity(const char* name) {
	entity* e = newEntity(name);
	if(!e) return NULL;

	// default fields
	appendField(e, transformNew(REN_TRANSFORM_NAME));
	appendField(e, cameraNew(REN_CAMERA_NAME));
	appendField(e, atmosphereNew(REN_ATMOSPHERE_NAME));

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

	// free yourself if not valid
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

			// free field
			if(tmp->vtable->free) tmp->vtable->free(tmp);
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

// checks if node is descendant of (supposed) parent
int isDescendant(entity* e, entity* parent) {
	// walk up
	while(e) {
		if(parent == e) return 1;
		e = e->parent;
	}

	return 0;
}

void moveChild(entity* e, entity* child) {
	if(e == child) return;

	// don't make cycles 
	if(isDescendant(e, child)) return;

	removeChild(child->parent, child);
	appendChild(e, child);
}

scene* ownerScene(entity* e) {
	if(!e) return NULL;

	// walk up
	while(e->parent) {
		e = e->parent;
	}

	// check for root
	if(strcmp(e->name, ROOT_NAME) != 0) return NULL;

	// is root
	return container_of(e, scene, root);			
}
