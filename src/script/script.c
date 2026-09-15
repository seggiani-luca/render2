#include "script.h"
#include "../scene/scene.h"
#include "lisp/lisp.h"
#include "lisp/execute.h"

// -- natives

value* nativeLog(environment* env, value* args) {
	// go through arguments
	while(args->type != VAL_NIL) {
		value* val = doEvaluateValue(env, args->cons.car);
		printValue(val);

		if(args->type != VAL_NIL) printf(" ");
		
		// advance
		args = args->cons.cdr;
	}
	printf("\n");
	
	return makeNil(arenaAlloc(&env->arena, sizeof(value)));
}

value* nativeWarn(environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Warn requires an argument");
		throw;
	}

	// take value 
	value* val = doEvaluateValue(env, args->cons.car);
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
	logEvent(WARN, EXEC, val->string);

	return makeNil(arenaAlloc(&env->arena, sizeof(value)));
}

value* nativeError(environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Error requires an argument");
		throw;
	}

	// take value 
	value* val = doEvaluateValue(env, args->cons.car);
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
	logEvent(ERROR, EXEC, val->string);
	throw;


	return makeNil(arenaAlloc(&env->arena, sizeof(value)));
}

// -- execution

void scriptStart(entity* ent) {
	INIT_JUMPS;
	
	// get script field
	field* fld = getField(ent, SCR_NAME);
	if(!fld) return;
	scriptField* scrField = (scriptField*)fld;

	// get script
	script* scr = scrField->ref->data;

	// get start hook
	envEntry* entry = queryEnvironment(scr->env, START_HOOK);
	if(!entry) {
		logEvent(WARN, EXEC, "Script on entity \"%s\" doesn't define start", 
			ent->name);
		return;
	}
	value* val = entry->value;
	if(val->type != VAL_FUNCTION) {
		logEvent(ERROR, EXEC, "Script on entity \"%s\" doesn't define start as function");
		return;
	}

	// call function
	try {
		applyFunction(scr->env, val->func, 
			makeNil(arenaAlloc(&scr->env->arena, sizeof(value))));
	} catch {
		logEvent(ERROR, EXEC, "Couldn't start script on entity \"%s\"", ent->name);
	}
	
	dumpEvents();
	
	RESTORE_JUMPS;
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

void scriptUpdate(entity* ent) {
	INIT_JUMPS;

	// get script field
	field* fld = getField(ent, SCR_NAME);
	if(!fld) return;
	scriptField* scrField = (scriptField*)fld;

	// get script
	script* scr = scrField->ref->data;

	// reset environment arena
	resetArena(&scr->env->arena);

	// get start hook
	envEntry* entry = queryEnvironment(scr->env, UPDATE_HOOK);
	if(!entry) {
		logEvent(WARN, EXEC, "Script on entity \"%s\" doesn't define update",
			ent->name);
		return;
	}
	value* val = entry->value;
	if(val->type != VAL_FUNCTION) {
		logEvent(ERROR, EXEC, "Script on entity \"%s\" doesn't define update as function");
		return;
	}

	// call function
	try {
		applyFunction(scr->env, val->func, 
			makeNil(arenaAlloc(&scr->env->arena, sizeof(value))));
	} catch {
		logEvent(ERROR, EXEC, "Couldn't update script on entity \"%s\"", ent->name);
	}

	dumpEvents();

	RESTORE_JUMPS;
}

void scriptsUpdate(entity* ent) {
	// update entity
	scriptUpdate(ent);
	
	// update children
	entity* cur = ent->child;
	while(cur) {
		scriptsUpdate(cur);
		cur = cur->peer;
	}
}
