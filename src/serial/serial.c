#include "serial.h"
#include "../scene/scene.h"
#include "json/json.h"
#include <stdlib.h>
#include <string.h>

// -- utils

// reads a file into a buffer
char* slurpBuffer(const char* path) {
	// open file
	FILE *f = fopen(path, "rb");
	if(!f) return NULL;

	// read file size
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	// allocate buffer
	char *buf = malloc(size + 1);

	// store file in buffer and terminate
	fread(buf, 1, size, f);
	buf[size] = '\0';

	// close file
	fclose(f);

	return buf;
}

// -- fields

jsonElement* intFieldSerialize(const field* f) {
	jsonElement* elem = newJsonNumber(((intField*)f)->val);
	return elem;
}

void intFieldDeserialize(field* f, const jsonElement* elem) {
	((intField*)f)->val = getJsonNumber(elem); 
}

jsonElement* floatFieldSerialize(const field* f) {
	jsonElement* elem = newJsonNumber(((floatField*)f)->val);
	return elem;
}

void floatFieldDeserialize(field* f, const jsonElement* elem) {
	((floatField*)f)->val = getJsonNumber(elem); 
}

jsonElement* stringFieldSerialize(const field* f) {
	jsonElement* elem = newJsonString(((stringField*)f)->str);
	return elem;
}

void stringFieldDeserialize(field* f, const jsonElement* elem) {
	// get original string
	const char* src = getJsonString(elem);	

	// copy over
	stringField* sf = (stringField*)f;
	strncpy(sf->str, src, ENT_STR_SIZ);
	sf->str[ENT_STR_SIZ - 1] = '\0';
}

// serializes a vector
jsonElement* vectorSerialize(void* val, int n) {
	jsonElement* elem = newJsonArray();

	// add each component
	for(int i = 0; i < n; i++) {
		addJsonAtTail(elem, newJsonNumber(
			*(float*)(val + i * sizeof(float))
		));
	}

	return elem;
}

// deserializes a vector
void vectorDeserialize(void* val, int n, const jsonElement* elem) {
	// get each component
	jsonElement* cur = getJsonHead(elem);
	for(int i = 0; i < n; i++) {
		*(float*)(val + i * sizeof(float)) = getJsonNumber(cur);
		
		cur = getJsonNext(cur);
	}
}

// macro for vector field serialization / deserialization
#define VEC_SERIAL_IMPL(n)                                               \
	jsonElement* float##n##FieldSerialize(const field* f) {              \
	    return vectorSerialize(&((float##n##Field*)f)->val, n);          \
	}                                                                    \
	void float##n##FieldDeserialize(field* f, const jsonElement* elem) { \
	    vectorDeserialize(&((float##n##Field*)f)->val, n, elem);         \
	}

// 2D vector field serialization / deserialization
VEC_SERIAL_IMPL(2)

// 3D vector field serialization / deserialization
VEC_SERIAL_IMPL(3)

// 4D vector field serialization / deserialization
VEC_SERIAL_IMPL(4)

// serializes a matrix
jsonElement* matrixSerialize(void* val, int n) {
	jsonElement* elem = newJsonArray();

	// add each entry
	for(int r = 0; r < n; r++) {
		jsonElement* row = newJsonArray();

		for(int c = 0; c < n; c++) {
			addJsonAtTail(row, newJsonNumber(
				*(float*)(val + (r + c * n) * sizeof(float))
			));
		}
		
		addJsonAtTail(elem, row);
	}

	return elem;
}

// deserializes a matrix
void matrixDeserialize(void* val, int n, const jsonElement* elem) {
	// get each entry 
	jsonElement* cur = getJsonHead(elem);
	for(int r = 0; r < n; r++) {
		jsonElement* ent = getJsonHead(cur);
		for(int c = 0; c < n; c++) {
			*(float*)(val + (r + c * n) * sizeof(float)) = getJsonNumber(ent);

			ent = getJsonNext(ent);
		}
		
		cur = getJsonNext(cur);
	}
}

// macro for matrix field serialization / deserialization
#define MAT_SERIAL_IMPL(n)                                              \
	jsonElement* mat##n##FieldSerialize(const field* f) {               \
	    return matrixSerialize(&((mat##n##Field*)f)->val, n);           \
	}                                                                   \
	void mat##n##FieldDeserialize(field* f, const jsonElement* elem) { \
	    matrixDeserialize(&((mat##n##Field*)f)->val, n, elem);          \
	}

// 2D matrix field serialization / deserialization
MAT_SERIAL_IMPL(2)

// 3D matrix field serialization / deserialization
MAT_SERIAL_IMPL(3)

// 4D matrix field serialization / deserialization
MAT_SERIAL_IMPL(4)

jsonElement* quatFieldSerialize(const field* f) {
	return float4FieldSerialize(f);
}

void quatFieldDeserialize(field* f, const jsonElement* elem) {
	float4FieldDeserialize(f, elem);
}

jsonElement* transformFieldSerialize(const field* f) {
	transformField* tf = (transformField*)f;
	jsonElement* elem = newJsonObject();

	// add position
	addJsonAtKey(elem, vectorSerialize(
		&tf->val.position, 3
	), "position");
	
	// add rotation
	addJsonAtKey(elem, vectorSerialize(
		&tf->val.rotation, 4
	), "rotation");

	// add scale 
	addJsonAtKey(elem, vectorSerialize(
		&tf->val.scale, 3
	), "scale");

	return elem;
}

void transformFieldDeserialize(field* f, const jsonElement* elem) {
	transformField* tf = (transformField*)f;

	// get position
	vectorDeserialize(&tf->val.position, 3, getJsonAtKey(
		elem, "position")
	);

	// get rotation 
	vectorDeserialize(&tf->val.rotation, 4, getJsonAtKey(
		elem, "rotation")
	);
	
	// get scale 
	vectorDeserialize(&tf->val.scale, 3, getJsonAtKey(
		elem, "scale")
	);
}

jsonElement* cameraFieldSerialize(const field* f) {
	cameraField* cf = (cameraField*)f;
	jsonElement* elem = newJsonObject();

	// add properties
	addJsonAtKey(elem, newJsonNumber(
		cf->val.fov
	), "fov");
	addJsonAtKey(elem, newJsonNumber(
		cf->val.nearPlane
	), "nearPlane");
	addJsonAtKey(elem, newJsonNumber(
		cf->val.farPlane
	), "farPlane");

	return elem;
}

void cameraFieldDeserialize(field* f, const jsonElement* elem) {
	cameraField* cf = (cameraField*)f;

	// get properties 
	cf->val.fov = getJsonNumber(getJsonAtKey(
		elem, "fov")
	);
	cf->val.nearPlane = getJsonNumber(getJsonAtKey(
		elem, "nearPlane")
	);
	cf->val.farPlane = getJsonNumber(
		getJsonAtKey(elem, "farPlane")
	);
}

jsonElement* atmosphereFieldSerialize(const field* f) {
	atmosphereField* af = (atmosphereField*)f;
	jsonElement* elem = newJsonObject();

	// add properties
	addJsonAtKey(elem, vectorSerialize(
		&af->val.ambient, 3
	), "ambient");
	addJsonAtKey(elem, newJsonString(
		af->val.ambientMap->path	
	), "cubemap");
	addJsonAtKey(elem, vectorSerialize(
		&af->val.sun, 3
	), "sun");
	addJsonAtKey(elem, vectorSerialize(
		&af->val.background, 3
	), "background");

	return elem;
}

void atmosphereFieldDeserialize(field* f, const jsonElement* elem) {
	atmosphereField* af = (atmosphereField*)f;

	// get properties 
	vectorDeserialize(&af->val.ambient, 3, getJsonAtKey(
		elem, "ambient")
	);
	const char* cubemapPath = getJsonString(getJsonAtKey(elem, "cubemap"));
	af->val.ambientMap = textureImport(cubemapPath);
	vectorDeserialize(&af->val.sun, 3, getJsonAtKey(
		elem, "sun")
	);
	vectorDeserialize(&af->val.background, 3, getJsonAtKey(
		elem, "background")
	);
}

jsonElement* textureFieldSerialize(const field* f) {
	textureField* tf = (textureField*)f;

	// add path
	jsonElement* elem = newJsonString(tf->ref->path);

	return elem;
}

void textureFieldDeserialize(field* f, const jsonElement* elem) {
	textureField* tf = (textureField*)f;

	// get path
	const char* path = getJsonString(elem);

	// import now
	tf->ref = textureImport(path);
}

jsonElement* meshFieldSerialize(const field* f) {
	meshField* mf = (meshField*)f;

	// add path
	jsonElement* elem = newJsonString(mf->ref->path);

	return elem;
}

void meshFieldDeserialize(field* f, const jsonElement* elem) {
	meshField* mf = (meshField*)f;

	// get path
	const char* path = getJsonString(elem);

	// import now
	mf->ref = meshImport(path);
}

jsonElement* materialFieldSerialize(const field* f) {
	materialField* mf = (materialField*)f;

	// add path
	jsonElement* elem = newJsonString(mf->ref->path);

	return elem;
}

void materialFieldDeserialize(field* f, const  jsonElement* elem) {
	materialField* mf = (materialField*)f;

	// get path
	const char* path = getJsonString(elem);

	// import now
	mf->ref = materialImport(path);
}

// typedef for field constructors
typedef field* (*fieldCtor)(const char* name);

// field tag - vtable map element
typedef struct  {
	// string tag of field
	char tag[32];

	// field vtable
	const fieldVtable* vtable;

	// field constructor
	fieldCtor ctor;
} serialTableElement;

// field tag - vtable map
serialTableElement serialTable[] = {
	"int",        &intFieldVtable,        intNew,
	"float",      &floatFieldVtable,      floatNew,
	"string",     &stringFieldVtable,     stringNew,
	"float2",     &float2FieldVtable,     float2New,
	"float3",     &float3FieldVtable,     float3New,
	"float4",     &float4FieldVtable,     float4New,
	"mat2",       &mat2FieldVtable,       mat2New,
	"mat3",       &mat3FieldVtable,       mat3New,
	"mat4",       &mat4FieldVtable,       mat4New,
	"quat",       &quatFieldVtable,       quatNew,
	"transform",  &transformFieldVtable,  transformNew,
	"camera",     &cameraFieldVtable,     cameraNew,
	"atmosphere", &atmosphereFieldVtable, atmosphereNew,
	"texture",    &textureFieldVtable,    textureNew,
	"mesh",       &meshFieldVtable,       meshNew,
	"material",   &materialFieldVtable,   materialNew
};

#define SERIAL_TABLE_SIZ (sizeof(serialTable) / sizeof(serialTableElement))

// maps a field to its tag
const char* getFieldTag(const field* fld) {
	// go through table
	for(size_t i = 0; i < SERIAL_TABLE_SIZ; i++) {
		serialTableElement* cur = &serialTable[i];
		if(cur->vtable == fld->vtable) return cur->tag;
	}

	return NULL;
}

// maps a field tag to its vtable, returning the constructor
const fieldVtable* getFieldVtable(const char* tag, fieldCtor* ctor) {
	// go through table
	for(size_t i = 0; i < SERIAL_TABLE_SIZ; i++) {
		serialTableElement* cur = &serialTable[i];
		if(strcmp(cur->tag, tag) == 0) {
			*ctor = cur->ctor;
			return cur->vtable;
		}
	}

	return NULL;
}

// serializes a field into a JSON element 
jsonElement* serializeField(const field* fld) {
	// build field object
	jsonElement* obj = newJsonObject();
	addJsonAtKey(obj, newJsonString(
		fld->name	
	), "name");
	addJsonAtKey(obj, newJsonString(
		getFieldTag(fld)
	), "type");

	// serialize field payload
	addJsonAtKey(obj, fld->vtable->serialize(
		fld	
	), "value");

	// return JSON
	return obj;
}

// deserializes a field from a JSON element 
field* deserializeField(jsonElement* elem) {
	// get constructor and vtable
	fieldCtor ctor = NULL;
	getFieldVtable(getJsonString(getJsonAtKey(elem, "type")), &ctor);

	// parse field object
	field* fld = ctor(getJsonString(getJsonAtKey(elem, "name")));

	// deserialize field payload
	fld->vtable->deserialize(fld, getJsonAtKey(elem, "value"));

	// return field
	return fld;
}

// -- entities

// serializes an entity into a JSON element
jsonElement* serializeEntity(const entity* ent) {
	// build entity object
	jsonElement* obj = newJsonObject();
	addJsonAtKey(obj, newJsonString(
		ent->name	
	), "name");

	// serialize each field
	jsonElement* fields = newJsonArray();
	addJsonAtKey(obj, fields, "fields");
	field* curField = ent->root;
	while(curField) {
		addJsonAtTail(fields, serializeField(
			curField
		));

		curField = curField->next;
	}
	
	// serialize each child
	jsonElement* children = newJsonArray();
	addJsonAtKey(obj, children, "children");
	entity* curChild = ent->child;
	while(curChild) {
		addJsonAtTail(children, serializeEntity(
			curChild		
		));

		curChild = curChild->peer;
	}

	// return JSON 
	return obj;
}

// deserializes an entity from a JSON element 
entity* deserializeEntity(jsonElement* elem) {
	// parse entity object
	entity* ent = newEntity(getJsonString(getJsonAtKey(elem, "name")));

	// call deserializeField for each field
	jsonElement* curField = getJsonHead(getJsonAtKey(elem, "fields"));
	while(curField) {
		field* fld = deserializeField(curField);
		appendField(ent, fld);

		curField = getJsonNext(curField);
	}
	
	// call deserializeEntity for each child
	jsonElement* curChild = getJsonHead(getJsonAtKey(elem, "children"));
	while(curChild) {
		entity* child = deserializeEntity(curChild);
		appendChild(ent, child);

		curChild = getJsonNext(curChild);
	}
	
	// return entity
	return ent;
}

// -- scenes

void serializeScene(const scene* scn, const char* path) {
	// build scene object
	jsonElement* obj = newJsonObject();
	addJsonAtKey(obj, newJsonString(
		scn->name
	), "name");
	
	// add all root entities
	jsonElement* root = newJsonArray();
	entity* cur = scn->root.child;
	while(cur) {
		addJsonAtTail(root, serializeEntity(cur));

		cur = cur->peer;
	}
	addJsonAtKey(obj, root, "root");

	// open file
	FILE *file = fopen(path, "wb");
	if(!file) return; 

	// serialize to file
	serializeJsonObject(file, obj);

	// cleanup
	freeJsonObject(obj);
	fclose(file);
}

void deserializeScene(scene* scn, const char* path){
	// file to buffer
	char* buf = slurpBuffer(path);
	if(!buf) return;

	// temporary pointer, for caller saving
	char* ptr = buf;

	// parse JSON from buffer
	jsonElement* obj = deserializeJsonObject(&ptr);

	// free render scene and set as dirty
	freeRenderScene(scn);
	scn->dirty = 1;
	
	// free scene
	freeEntityChildren(&scn->root);

	// parse scene object
	const char* name = getJsonString(getJsonAtKey(obj, "name"));

	// copy over
	strncpy(scn->name, name, ENT_NAME_SIZ);
	scn->name[ENT_NAME_SIZ - 1] = '\0';

	// rebuild root
	jsonElement* cur = getJsonHead(getJsonAtKey(obj, "root"));
	while(cur) {
		appendChild(&scn->root, deserializeEntity(cur));

		cur = getJsonNext(cur);
	}

	// cleanup
	freeJsonObject(obj);
	free(buf);
}

const char* getScenePath(const char* name) {
	// static path string
	static char path[DAT_PATH_SIZ];
	path[0] = '\0';

	// build path name
	strcat(path, SCENE_DIR);
	strcat(path, name);
	strcat(path, SCENE_EXT);

	// remove silly spaces
	for(int i = 0; i < DAT_PATH_SIZ; i++) {
		if(path[i] == ' ') path[i] = '_';
	}

	return path;
}
