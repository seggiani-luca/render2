#ifndef SCENE_ENTITY_H
#define SCENE_ENTITY_H

#include "../../gui/gui.h"
#include "../../math/math.h"

// -- fields

// size of entity and field names
#define ENT_NAME_SIZ 64

// size of string fields
#define ENT_STR_SIZ 64

// forward declaration for fieldVtable
typedef struct field field;

// vtable of methods for field access operations
typedef struct {
	// read method
	void (*read)(const field* f, void* dst);

	// write method
	void (*write)(field* f, const void* src);

	// debug print method
	void (*print)(const field* f);

	// GUI rendering method (pushes quads to a GUI queue)
	int (*gui)(const field* f, guiContext* ctx);
} fieldVtable;

// basic field data
struct field {
	// name of field
	char name[ENT_NAME_SIZ];

	// vtable of methods for access operations on this field
	fieldVtable* vtable;

	// next field in field list
	field* next;
};

// shortcut for field reading via own method
void readField(const field* f, void* dst);

// shortcut for field writing via own method
void writeField(field* f, const void* src);

// shortcut for field debug printing via own method
void printField(const field* f);

// shortcut for field gui rendering via own method
int guiField(const field* f, guiContext* ctx);

// -- integer field

// integer field data
typedef struct {
	field base;

	// integer data
	int val;
} intField;

// creates a new integer field
field* intNew(const char* name);

// -- float field

// float field data
typedef struct {
	field base;

	// float data
	float val;
} floatField;

// creates a new float field
field* floatNew(const char* name);

// -- string field

// string field data
typedef struct {
	field base;

	// string data
	char str[ENT_STR_SIZ];
} stringField;

// creates a new string field
field* stringNew(const char* name);

// -- vector fields 

// generic vector field data
#define VEC_FIELD_DECL(n)                   \
	typedef struct {                        \
	    field base;                         \
	    float##n val;                       \
	} float##n##Field;                      \
	                                        \
	field* float##n##New(const char* name);

// 2D vector field
VEC_FIELD_DECL(2)

// 3D vector field
VEC_FIELD_DECL(3)

// 4D vector field
VEC_FIELD_DECL(4)

// -- entities

// entity data
struct entity {
	// name of entity
	char name[ENT_NAME_SIZ];

	// root of field list
	field* root;

	// number of fields
	int fieldCount;

	// parent of this entity (NULL for root)
	struct entity* parent;

	// direct children of this entity
	struct entity* child;

	// next peer of this entity
	struct entity* peer;

	// number of children
	int childCount;
};
typedef struct entity entity;

// debug prints an entity
void printEntity(const entity* e);

// -- lifetime

// creates a new entity
entity* newEntity(const char* name);

// frees an entity
void freeEntity(entity* e);

// frees an entity's children
void freeEntityChildren(entity* e);

// -- fields

// appends a field to an entity
void appendField(entity* e, void* f);

// removes first field from an entity, by name
void removeField(entity* e, const char* name);

// gets first field in an entity, by name
field* getField(const entity* e, const char* name);

// -- hierarchy

// appends a child to an entity
void appendChild(entity* e, entity* child);

// removes a child from an entity
void removeChild(entity* e, entity* child);

// moves a child from an entity to another
void moveChild(entity* e, entity* child);

#endif
