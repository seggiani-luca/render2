#ifndef ENTITY_H
#define ENTITY_H

#include "../gui/gui.h"

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

	// GUI rendering method (pushes quads too a GUI queue)
	void (*gui)(const field* f, guiContext* ctx);
} fieldVtable;

// basic field data
struct field {
	// name of field
	char name[ENT_STR_SIZ];

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
void guiField(const field* f, guiContext* ctx);

// -- integer field

// integer field data
typedef struct {
	field base;

	// integer data
	int val;
} intField;

// creates a new integer field
intField* intNew(const char* name, int val);

// -- float field

// float field data
typedef struct {
	field base;

	// float data
	float val;
} floatField;

// creates a new float field
floatField* floatNew(const char* name, float val);

// -- string field

// string field data
typedef struct {
	field base;

	// string data
	char str[ENT_STR_SIZ];
} stringField;

// creates a new string field
stringField* stringNew(const char* name, const char* val);

// -- entities

// entity data
struct entity {
	// name of entity
	char name[ENT_STR_SIZ];

	// root of field list
	field* root;
};
typedef struct entity entity;

// creates a new entity
entity* newEntity(const char* name);

// frees an entity
void freeEntity(entity* e);

// appends a field to an entity
void appendField(entity* e, void* f);

// removes first field from an entity, by name
void removeField(entity* e, const char* name);

// gets first field in an entity, by name 
field* getField(const entity* e, const char* name);

// debug prints an entity
void printEntity(const entity* e);

#endif
