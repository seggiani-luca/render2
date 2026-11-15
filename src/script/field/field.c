#include "field.h"
#include "../lisp/execute.h"
#include "../../scene/scene.h"
#include <string.h>

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

// gets a vector
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

// sets a vector
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
