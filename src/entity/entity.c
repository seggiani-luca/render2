#include "entity.h"
#include "../gui/inspector/inspector.h"
#include <stdlib.h>
#include <stdio.h>
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
int guiField(const field* f, guiContext* ctx) { 
	return f->vtable->gui(f, ctx); 
}

// macro for vtable declaration
#define VTABLE(type) \
	fieldVtable type##FieldVtable = { \
	    .read  = type##Read,          \
	    .write = type##Write,         \
	    .print = type##Print,         \
	    .gui   = type##FieldGui,      \
	};                                

// macro for field allocation
#define ALLOC_FIELD(type) \
	if(*name == '\0') return NULL;                \
	type##Field* f = malloc(sizeof(type##Field)); \
	if(!f) return NULL;                           \
	f->base = newField(name, &type##FieldVtable);

// -- integer field

// reads an integer field
void intRead(const field* f, void* dst) {
	*(int*) dst = ((intField*) f)->val; 
}

// writes an integer field
void intWrite(field* f, const void* src) {
	((intField*) f)->val = *(int*) src;
}

// debug prints an integer field
void intPrint(const field* f) {
	printf("%s (Integer): %d", f->name, ((intField*) f)->val);
}

VTABLE(int)

field* intNew(const char* name) {
	ALLOC_FIELD(int)
	f->val = 0;

	return (field*) f;
}

// -- float field

// reads a float field
void floatRead(const field* f, void* dst) {
	*(float*) dst = ((floatField*) f)->val; 
}

// writes a float field
void floatWrite(field* f, const void* src) {
	((floatField*) f)->val = *(float*) src;
}

// debug prints a float field
void floatPrint(const field* f) {
	printf("%s (Float): %f", f->name, ((floatField*) f)->val);
}

VTABLE(float)

field* floatNew(const char* name) {
	ALLOC_FIELD(float)
	f->val = 0.0f;

	return (field*) f;
}

// -- string field

// reads a string field
void stringRead(const field* f, void* dst) {
	*(char**) dst = ((stringField*) f)->str; 
}

// writes a string field
void stringWrite(field* f, const void* src) {
	stringField* sf = (stringField*) f;
	strncpy(sf->str, *(char**) src, ENT_STR_SIZ);
	sf->str[ENT_STR_SIZ - 1] = '\0';
}

// debug prints a string field
void stringPrint(const field* f) {
	printf("%s (String): %s", f->name, ((stringField*) f)->str);
}

VTABLE(string)

field* stringNew(const char* name) {
	ALLOC_FIELD(string)
	*f->str = '\0';

	return (field*) f;
}

// -- entities

void printEntity(const entity* e) {
	printf("%s (Entity):\n", e->name);
	field* cur = e->root;
	while(cur) {
		printf("\t"); printField(cur); printf("\n");

		cur = cur->next;
	}
}

// -- lifetime

entity* newEntity(const char* name) {
	if(*name == '\0') return NULL;

	// allocate entity
	entity* e = malloc(sizeof(entity));
	if(!e) return NULL;

	// copy name and clear root
	strncpy(e->name, name, ENT_NAME_SIZ);
	e->name[ENT_NAME_SIZ - 1] = '\0';
	e->root = NULL;
	e->fieldCount = 0;

	// setup hierarchy
	e->parent = e->child = e->peer = NULL;

	return e;
}

void freeEntity(entity* e) {
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
	((field*) f)->next = NULL;
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

// appends a child to an entity 
void appendChild(entity* e, entity* child) {
	child->parent = e;
	
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
	// simple on first child
	if (e->child == child) {
		e->child = child->peer;
		child->parent = NULL;
		child->peer = NULL;
		return;
	}
	
	// walk all children
	entity* temp = e->child;
	while(temp->peer && temp->peer != child) temp = temp->peer;

	// remove only if found
	if (!temp || temp->peer != child) return;
	temp->peer = child->peer;
	child->parent = NULL;
	child->peer = NULL;
}

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

void freeScene(scene* s) {
	// TODO implement
}

sceneIter getScIter(scene* s) {
	return (sceneIter) {
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
	while (it->cur && !it->cur->peer) {
		it->depth--;
		it->cur = it->cur->parent;
	}

	// return NULL when root reached
	if(!it->cur) return NULL;

	// go to peer
	it->cur = it->cur->peer;
	return it->cur;
}
