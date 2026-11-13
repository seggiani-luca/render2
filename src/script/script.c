#include "script.h"
#include "lisp/execute.h"
#include "../input/input.h"
#include "../scene/scene.h"
#include "../exception/exception.h"
#include "lisp/lisp.h"
#include <stdio.h>
#include <string.h>

// -- fields

value* intFieldGet(arena* a, const field* f) {
	value* val = makeNumber(a, ((intField*)f)->val);
	return val;
}

void intFieldSet(field* f, const value* val) {
	if(val->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Integer conversion requires number");
		throw;
	}

	((intField*)f)->val = val->number;
}

value* floatFieldGet(arena* a, const field* f) {
	value* val = makeNumber(a, ((floatField*)f)->val);
	return val;
}

void floatFieldSet(field* f, const value* val) {
	if(val->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Float conversion requires number");
		throw;
	}

	((floatField*)f)->val = val->number;
}

value* stringFieldGet(arena* a, const field* f) {
	value* val = makeString(a, ((stringField*)f)->str);
	return val;
}

void stringFieldSet(field* f, const value* val) {
	if(val->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "String conversion requires string");
		throw;
	}

	// copy over
	stringField* sf = (stringField*)f;
	strncpy(sf->str, val->string, ENT_STR_SIZ);
	sf->str[ENT_STR_SIZ - 1] = '\0';
}

// sets a vector
value* vectorGet(arena* a, void* vec, int n) {
	value* list = NULL;
	value** tail = &list;

	// add each component
	float* values = vec;

	for(int i = 0; i < n; i++) {
		value* val = makeNumber(a, values[i]);
		
		// concatenate to list
		value* node = makeCons(a,
			val,
			makeNil()
		);

		// append node
		*tail = node;
		tail = &node->cons.cdr;

	}

	return list;
}

// gets a vector
void vectorSet(void* vec, int n, const value* val) {
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector must be list");
		throw;
	}

	// get each component
	float* values = vec;

	for(int i = 0; i < n; i++) {
        if(val->type != VAL_CONS) {
            logEvent(ERROR, EXEC, "Vector requires %d elements", n);
            throw;
        }

		value* elem = val->cons.car;
		if(elem->type != VAL_NUMBER) {
			logEvent(ERROR, EXEC, "Vector element must be number");
			throw;
		}

		values[i] = elem->number;
	
		// advance
		val = val->cons.cdr;
	}

	if(val->type != VAL_NIL) {
        logEvent(ERROR, EXEC, "Vector must contain exactly %d elements", n);
        throw;
    }
}

// macro for vector field serialization / deserialization
#define VEC_VALUE_IMPL(n)                                    \
	value* float##n##FieldGet(arena* a, const field* f) {    \
	    return vectorGet(a, &((float##n##Field*)f)->val, n); \
	}                                                        \
	void float##n##FieldSet(field* f, const value* val) {    \
	    vectorSet(&((float##n##Field*)f)->val, n, val);      \
	}

// 2D vector field serialization / deserialization
VEC_VALUE_IMPL(2)

// 3D vector field serialization / deserialization
VEC_VALUE_IMPL(3)

// 4D vector field serialization / deserialization
VEC_VALUE_IMPL(4)

// serializes a matrix
value* matrixGet(arena* a, void* vec, int n) {
	value* list = NULL;
	value** rowTail = &list;

	// add each row
    float* values = vec;

    for(int r = 0; r < n; r++) {
		// create row
		value* row = NULL;
		value** colTail = &row;

        for(int c = 0; c < n; c++) {
			value* val = makeNumber(a, values[r + c * n]);
		
			// concate to row 
			value* colNode = makeCons(a,
				val,
				makeNil()
			);

			// append node 
			*colTail = colNode;
			colTail = &colNode->cons.cdr;
        }

		// concate to list
		value* rowNode = makeCons(a,
			row,
			makeNil()
		);

		// append row
		*rowTail = rowNode;
		rowTail = &rowNode->cons.cdr;
    }

    return list;
}

// deserializes a matrix
void matrixSet(void* vec, int n, const value* val) {
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Matrix must be list");
		throw;
	}

    // get each entry
    float* values = vec;

	// go through rows
    for(int r = 0; r < n; r++) {
        if(val->type != VAL_CONS) {
            logEvent(ERROR, EXEC, "Matrix requires %d rows", n);
            throw;
        }

		value* row = val->cons.car;

		// go through columns (of row)
        for(int c = 0; c < n; c++) {
			if(row->type != VAL_CONS) {
				logEvent(ERROR, EXEC, "Matrix requires %d columns", n);
				throw;
			}

			value* ent = row->cons.car;
			if(ent->type != VAL_NUMBER) {
				logEvent(ERROR, EXEC, "Matrix entry must be number");
				throw;
			}

			values[r + c * n]  = ent->number;
		
			// advance
			row = row->cons.cdr;
		}
	
		if(row->type != VAL_NIL) {
			logEvent(ERROR, EXEC, "Matrix must contain exactly %d columns", n);
			throw;
		}
	
		// advance
		val = val->cons.cdr;
	}

	if(val->type != VAL_NIL) {
        logEvent(ERROR, EXEC, "Matrix must contain exactly %d rows", n);
        throw;
    }
}

// macro for matrix field serialization / deserialization
#define MAT_SERIAL_IMPL(n)                                 \
	value* mat##n##FieldGet(arena* a, const field* f) {    \
	    return matrixGet(a, &((mat##n##Field*)f)->val, n); \
	}                                                      \
	void mat##n##FieldSet(field* f, const value* val) {    \
	    matrixSet(&((mat##n##Field*)f)->val, n, val);      \
	}

// 2D matrix field serialization / deserialization
MAT_SERIAL_IMPL(2)

// 3D matrix field serialization / deserialization
MAT_SERIAL_IMPL(3)

// 4D matrix field serialization / deserialization
MAT_SERIAL_IMPL(4)

value* quatFieldGet(arena* a, const field* f) {
	return float4FieldGet(a, f);
}

void quatFieldSet(field* f, const value* val) {
	float4FieldSet(f, val);
}

value* transformFieldGet(arena* a, const field* f) {
	transformField* tf = (transformField*)f;
	
	value* pos = vectorGet(a, &tf->val.position, 3);
	value* rot = vectorGet(a, &tf->val.rotation, 4);
	value* scale = vectorGet(a, &tf->val.scale, 3);

	return makeCons(a, pos,
		makeCons(a,    rot,
		makeCons(a,    scale,
		makeNil())));
}

void transformFieldSet(field* f, const value* val) {
	transformField* tf = (transformField*)f;

	// set position
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Transform requires position");
		throw;
	}
	value* pos = val->cons.car;

	vectorSet(&tf->val.position, 3, pos);

	val = val->cons.cdr;
	
	// set rotation 
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Transform requires rotation");
		throw;
	}
	value* rot = val->cons.car;

	vectorSet(&tf->val.rotation, 4, rot);

	val = val->cons.cdr;

	// set scale 
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Transform requires scale");
		throw;
	}
	value* scale = val->cons.car;

	vectorSet(&tf->val.scale, 3, scale);

	val = val->cons.cdr;

	if(val->type != VAL_NIL) {
        logEvent(ERROR, EXEC, "Transform must contain exactly 3 elements");
        throw;
    }
}

value* cameraFieldGet(arena* a, const field* f) {
	cameraField* cf = (cameraField*)f;
	
	value* far = makeNumber(a, cf->val.farPlane);
	value* near = makeNumber(a, cf->val.nearPlane);
	value* fov = makeNumber(a, cf->val.fov);

	return makeCons(a, far,
		makeCons(a,    near,
		makeCons(a,    fov,
		makeNil())));
}

void cameraFieldSet(field* f, const value* val) {
	cameraField* cf = (cameraField*)f;

	// set far 
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Camera requires far plane");
		throw;
	}
	value* far = val->cons.car;

	if(far->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Camera far plane must be number");
		throw;
	}
	cf->val.farPlane = far->number;

	val = val->cons.cdr;

	// set near
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Camera requires near plane");
		throw;
	}
	value* near = val->cons.car;

	if(near->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Camera near plane must be number");
		throw;
	}
	cf->val.nearPlane = near->number;

	val = val->cons.cdr;

	// set FOV
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Camera requires FOV");
		throw;
	}
	value* fov = val->cons.car;

	if(fov->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Camera FOV must be number");
		throw;
	}
	cf->val.fov = fov->number;

	val = val->cons.cdr;
	
	if(val->type != VAL_NIL) {
        logEvent(ERROR, EXEC, "Camera must contain exactly 3 elements");
        throw;
    }
}

value* atmosphereFieldGet(arena* a, const field* f) {
	atmosphereField* af = (atmosphereField*)f;

	value* amb = vectorGet(a, &af->val.ambient, 3);
	value* sun = vectorGet(a, &af->val.sun, 3);
	value* bg = vectorGet(a, &af->val.background, 3);

	// TODO handle references

	return makeCons(a, amb,
		makeCons(a,    sun,
		makeCons(a,    bg,
		makeNil())));
}

void atmosphereFieldSet(field* f, const value* val) {
	atmosphereField* af = (atmosphereField*)f;

	// set ambient 
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Atmosphere requires ambient");
		throw;
	}
	value* amb = val->cons.car;

	vectorSet(&af->val.ambient, 3, amb);

	val = val->cons.cdr;
	
	// set sun
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Atmosphere requires sun");
		throw;
	}
	value* sun = val->cons.car;

	vectorSet(&af->val.sun, 3, sun);

	val = val->cons.cdr;

	// set background 
	if(val->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Atmosphere requires background");
		throw;
	}
	value* bg = val->cons.car;

	vectorSet(&af->val.background, 3, bg);

	val = val->cons.cdr;

	// TODO handle references

	if(val->type != VAL_NIL) {
        logEvent(ERROR, EXEC, "Atmosphere must contain exactly 3 elements");
        throw;
    }	
}

value* textureFieldGet(arena* a, const field* f) {
	// TODO handle references
	return makeNil();
}

void textureFieldSet(field* f, const value* val) {
	// TODO handle references
}

value* meshFieldGet(arena* a, const field* f) {
	// TODO handle references
	return makeNil();
}

void meshFieldSet(field* f, const value* val) {
	// TODO handle references
}

value* materialFieldGet(arena* a, const field* f) {
	// TODO handle references
	return makeNil();
}

void materialFieldSet(field* f, const value* val) {
	// TODO handle references
}

value* scriptFieldGet(arena* a, const field* f) {
	// TODO handle references
	return makeNil();
}

void scriptFieldSet(field* f, const value* val) {
	// TODO handle references
}

value* shaderFieldGet(arena* a, const field* f) {
	// TODO handle references
	return makeNil();
}

void shaderFieldSet(field* f, const value* val) {
	// TODO handle references
}

// -- natives

// current entity
entity* curEnt = NULL;

value* nativeLog(arena* a, environment* env, value* args) {
	// go through arguments
	while(args->type != VAL_NIL) {
		value* val = evaluateValue(a, env, args->cons.car);
		printValue(val);
		
		// advance
		args = args->cons.cdr;
		if(args->type != VAL_NIL) printf(" ");
	}
	printf("\n");
	
	return makeNil();
}

value* nativeWarn(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Warn requires an argument");
		throw;
	}

	// take value 
	value* val = evaluateValue(a, env, args->cons.car);
	if(val->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Warn takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Warn takes exactly one argument");
		throw;
	}

	// error
	logEvent(WARN, EXEC, "%s", val->string);

	return makeNil();
}

value* nativeError(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Error requires an argument");
		throw;
	}

	// take value 
	value* val = evaluateValue(a, env, args->cons.car);
	if(val->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Error takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Error takes exactly one argument");
		throw;
	}

	// error
	logEvent(ERROR, EXEC, "%s", val->string);
	throw;

	return makeNil();
}

value* nativeGetKey(arena* a, environment* env, value* args) {
	// exactly two arguments
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-key requires an argument");
		throw;
	}

	// take needed state
	value* neededState = evaluateValue(a, env, args->cons.car);
	if(neededState->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Get-key takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-key requires an argument");
		throw;
	}

	// take needed key 
	value* neededKey = evaluateValue(a, env, args->cons.car);
	if(neededKey->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Get-key takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-key takes exactly two arguments");
		throw;
	}

	if(strcmp(neededState->string, KEY_PRESS_ALIAS) != 0
	&& strcmp(neededState->string, KEY_RELES_ALIAS) != 0
	&& strcmp(neededState->string, KEY_DOWN_ALIAS)  != 0) {
		logEvent(WARN, EXEC, "Unknown state in get-key");
		return makeBool(a, 0);
	}

	// get input
	inputState* local = inputGet();

	// query key
	int i = 0;
	int j = 0;
	for(;;) {
		inputKeyName name;
		if(j == NUM_MOUSE_BUTTONS) break;
		if(i == NUM_KEYS) name = mouseButtonNames[j++];
		else name = keyNames[i++];

		if(strcmp(neededKey->string, name.name) == 0) {
			keyState keyState = (i == NUM_KEYS) ?
				local->mouse.buttons[name.code] :
				local->keys[name.code];

			if(strcmp(neededState->string, KEY_PRESS_ALIAS) == 0) 
				return makeBool(a, keyState.press);
			if(strcmp(neededState->string, KEY_RELES_ALIAS) == 0)
				return makeBool(a, keyState.reles);
			if(strcmp(neededState->string, KEY_DOWN_ALIAS) == 0)
				return makeBool(a, keyState.down);

			break;
		}
	}

	logEvent(WARN, EXEC, "Unknown key in get-key");
	return makeBool(a, 0);
}

value* nativeGetAxis(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-axis requires an argument");
		throw;
	}

	// take needed axis 
	value* neededAxis = evaluateValue(a, env, args->cons.car);
	if(neededAxis->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Get-axis takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-axis takes exactly one argument");
		throw;
	}

	// get input
	inputState* local = inputGet();

	// query axis
	if(strcmp(neededAxis->string, MOUSE_X_ALIAS) == 0)
		return makeNumber(a, local->mouse.x);
	if(strcmp(neededAxis->string, MOUSE_Y_ALIAS) == 0)
		return makeNumber(a, local->mouse.y);
	if(strcmp(neededAxis->string, MOUSE_SCROLL_ALIAS) == 0)
		return makeNumber(a, local->mouse.scroll);

	logEvent(WARN, EXEC, "Unknown axis in get-axis");
	return makeNil();
}

// converts entity ID to float
float entToFloat(uint32_t id) {
	return *(float*) &id;
}

// converts float to entity ID
uint32_t floatToEnt(float fl) {
	return *(uint32_t*) &fl;
}

value* nativeFindEntity(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Find-entity requires an argument");
		throw;
	}

	// take entity name 
	value* name = evaluateValue(a, env, args->cons.car);
	if(name->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Find-entity takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Find-entity takes exactly one argument");
		throw;
	}

	// find entity
	if(strcmp(name->string, SELF_NAME) == 0) {
		return makeNumber(a, entToFloat(curEnt->id));
	}
	
	// owner scene
	scene* scn = ownerScene(curEnt);

	// go through all entities
	sceneIter iter = getScIter(scn);
	do {
		if(strcmp(iter.cur->name, name->string) == 0)
		return makeNumber(a, entToFloat(iter.cur->id));
	} while(scIterNext(&iter));

	return makeNil();
}

value* nativeGetEntityName(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-entity-name requires an argument");
		throw;
	}

	// take entity
	value* id = evaluateValue(a, env, args->cons.car);
	if(id->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Get-entity-name takes a number argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-entity-name takes exactly one argument");
		throw;
	}

	// get entity
	entity* ent = entityFromId(floatToEnt(id->number));
	if(ent) {
		return makeString(a, ent->name);
	}

	return makeNil();
}

value* nativeGetEntityChild(arena* a, environment* env, value* args) {
	// exactly two arguments
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-entity-child requires an argument");
		throw;
	}

	// take entity
	value* id = evaluateValue(a, env, args->cons.car);
	if(id->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Get-entity-child takes a number first argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-entity-child requires an argument");
		throw;
	}

	// take child 
	value* name = evaluateValue(a, env, args->cons.car);
	if(name->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Get-entity-child takes a string second argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-entity-child takes exactly one argument");
		throw;
	}

	// get entity
	entity* ent = entityFromId(floatToEnt(id->number));
	if(!ent) return makeNil();

	entity* child = ent->child;
	while(child) {
		if(strcmp(child->name, name->string) == 0)
			return makeNumber(a, entToFloat(child->id));

		child = child->peer;
	}

	return makeNil();
}

value* nativeGetEntityChildren(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-entity-children requires an argument");
		throw;
	}

	// take entity
	value* id = evaluateValue(a, env, args->cons.car);
	if(id->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Get-entity-children takes a number argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-entity-children takes exactly one argument");
		throw;
	}

	// get entity
	entity* ent = entityFromId(floatToEnt(id->number));
	if(!ent) return makeNil();

	// build children list
	value* list = makeNil();
	entity* child = ent->child;
	while(child) {
		list = makeCons(a, makeNumber(a, entToFloat(child->id)), list);
		child = child->peer;
	}

	return list;
}

value* nativeGetEntityParent(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-entity-parent requires an argument");
		throw;
	}

	// take entity
	value* id = evaluateValue(a, env, args->cons.car);
	if(id->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Get-entity-parent takes a number argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-entity-parent takes exactly one argument");
		throw;
	}

	// get entity
	entity* ent = entityFromId(floatToEnt(id->number));
	if(ent && ent->parent) {
		return makeNumber(a, entToFloat(ent->parent->id));
	}

	return makeNil();
}

value* nativeGetField(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Get-field requires an argument");
		throw;
	}

	// take field name
	value* name = evaluateValue(a, env, args->cons.car);
	if(name->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Get-field takes a string argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-field takes exactly one argument");
		throw;
	}

	// get current entity
	if(!curEnt) return makeNil();

	// get field
	field* fld = getField(curEnt, name->string);
	if(!fld) return makeNil();

	return getFieldValue(a, fld);
}

value* nativeSetField(arena* a, environment* env, value* args) {
	// exactly two arguments
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Set-field requires an argument");
		throw;
	}

	// take field name
	value* name = evaluateValue(a, env, args->cons.car);
	if(name->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Set-field takes a string first argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Set-field requires a second argument");
		throw;
	}

	// take value
	value* val = evaluateValue(a, env, args->cons.car);

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Set-field takes exactly two arguments");
		throw;
	}

	// get current entity
	if(!curEnt) return makeNil();

	// get field
	field* fld = getField(curEnt, name->string);
	if(!fld) return makeNil();

	setFieldValue(fld, val);

	return makeNil();
}

value* nativeRequireField(arena* a, environment* env, value* args) {
	// exactly two arguments
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Require-field requires an argument");
		throw;
	}

	// take field 
	value* typ = evaluateValue(a, env, args->cons.car);
	if(typ->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Require-field takes a string first argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Require-field requires an argument");
		throw;
	}

	// take name 
	value* name = evaluateValue(a, env, args->cons.car);
	if(name->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Require-field takes a string second argument");
		throw;
	}

	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Require-field takes exactly two arguments");
		throw;
	}

	// get constructor
	fieldCtor ctor = NULL;
	for(size_t i = 0; i < FIELD_CTOR_TABLE_SIZ; i++) {
		ctorTableElement* cur = &fieldCtorTable[i];
		if(strcmp(cur->tag, typ->string) == 0) ctor = cur->ctor;
	}
	if(!ctor) {
		logEvent(WARN, EXEC, "Required field type unknown");
		return makeNil();
	} 

	// create field 
	appendField(curEnt, ctor(name->string));

	return makeNil();
}

// custom native environment entries helper
nativeEnvEntry customNativeEntriesHelper[NUM_CUSTOM_NATIVE_ENTRIES] = {
	// engine hooks
	
	// logging
	{ .key = "log",      .fn = nativeLog   },
	{ .key = "warn",     .fn = nativeWarn  },
	{ .key = "error",    .fn = nativeError },

	// input
	{ .key = "get-key",  .fn = nativeGetKey  },
	{ .key = "get-axis", .fn = nativeGetAxis },

	// entity handling
	{ .key = "find-entity",          .fn = nativeFindEntity        },
	{ .key = "get-entity-name",      .fn = nativeGetEntityName     },
	{ .key = "get-entity-child",     .fn = nativeGetEntityChild    },
	{ .key = "get-entity-children",  .fn = nativeGetEntityChildren },
	{ .key = "get-entity-parent",    .fn = nativeGetEntityParent   },

	// field handling
	{ .key = "get-field",            .fn = nativeGetField },
	{ .key = "set-field",            .fn = nativeSetField },
	{ .key = "require-field",        .fn = nativeRequireField },
};

// -- execution

// populate script environment
void populateEnvironment(float deltaTime, script* scr) {
	value* deltaTimeVal = makeNumber(&scr->ctx->execArena, deltaTime);
	addToFrame(scr->ctx->env->scriptFrame, DELTA_TIME_KEY, deltaTimeVal);

	value* selfEntity = makeNumber(&scr->ctx->execArena, entToFloat(curEnt->id));
	addToFrame(scr->ctx->env->scriptFrame, SELF_NAME, selfEntity);
}

void scriptStart(entity* ent) {
	INIT_JUMPS;
	
	curEnt = ent;

	// go through all fields
	field* cur = ent->root;
	while(cur) {
		// advance first
		field* fld = cur;
		cur = cur->next;

		// is script?
		if(fld->vtable != &scriptFieldVtable) continue;
		scriptField* scrField = (scriptField*)fld;
		if(!scrField->ref) continue;
		
		// get script
		script* scr = scrField->ref->data;
		rewindScript(scr->ctx);
		
		// populate environment
		populateEnvironment(0.0f, scr);

		// start hook
		if(!evaluateFuncFromScript(scr->ctx, START_HOOK))
			logEvent(ERROR, EXEC, "Couldn't start entity \"%s\"", ent->name);
	}
}

void scriptsStart(entity* ent) {
	// start entity
	scriptStart(ent);
	
	// start children
	entity* cur = ent->child;
	while(cur) {
		scriptsStart(cur);
		cur = cur->peer;
	}
}

void scriptUpdate(entity* ent, float deltaTime) {
	INIT_JUMPS;

	curEnt = ent;

	// go through all fields
	field* cur = ent->root;
	while(cur) {
		// advance first
		field* fld = cur;
		cur = cur->next;

		// is script?
		if(fld->vtable != &scriptFieldVtable) continue;
		scriptField* scrField = (scriptField*)fld;
		if(!scrField->ref) continue;
		
		// get script
		script* scr = scrField->ref->data;
		rewindScript(scr->ctx);

		// populate environment
		populateEnvironment(deltaTime, scr);

		// start hook
		if(!evaluateFuncFromScript(scr->ctx, UPDATE_HOOK))
			logEvent(ERROR, EXEC, "Couldn't update entity \"%s\"", ent->name);
	}
}

void scriptsUpdate(entity* ent, float deltaTime) {
	// update entity
	scriptUpdate(ent, deltaTime);
	
	// update children
	entity* cur = ent->child;
	while(cur) {
		scriptsUpdate(cur, deltaTime);
		cur = cur->peer;
	}
}
