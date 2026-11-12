#include "script.h"
#include "lisp/lisp.h"
#include "lisp/execute.h"
#include "../input/input.h"
#include "../scene/scene.h"
#include "../exception/exception.h"
#include <stdio.h>
#include <string.h>

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

	// take child 
	value* name = evaluateValue(a, env, args->cons.car);
	if(name->type != VAL_STRING) {
		logEvent(ERROR, EXEC, "Get-entity-child takes a string second argument");
		throw;
	}

	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Get-entity-name takes exactly one argument");
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
		logEvent(ERROR, EXEC, "Get-entity-childrenirequires an argument");
		throw;
	}

	// take entity
	value* id = evaluateValue(a, env, args->cons.car);
	if(id->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Get-entity-child takes a number argument");
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

// -- execution

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
		value* deltaTimeVal = makeNumber(&scr->ctx->execArena, deltaTime);
		addToFrame(scr->ctx->env->scriptFrame, DELTA_TIME_KEY, deltaTimeVal);

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
