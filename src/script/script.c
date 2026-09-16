#include "script.h"
#include "../scene/entity/entity.h"
#include "lisp/lisp.h"
#include "lisp/execute.h"
#include "../exception/exception.h"
#include <stdio.h>

// -- natives

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

// -- execution

void scriptStart(entity* ent) {
	INIT_JUMPS;
	
	// get script field
	field* fld = getField(ent, SCR_NAME);
	if(!fld) return;
	scriptField* scrField = (scriptField*)fld;
	if(!scrField->ref) return;

	// get script
	script* scr = scrField->ref->data;

	// start hook
	if(!evaluateFuncFromScript(scr->ctx, START_HOOK)) {
		logEvent(ERROR, EXEC, "Couldn't start entity \"%s\"", ent->name);
		dumpEvents();
	}
}

void scriptsStart(entity* ent) {
	// start entity
	scriptStart(ent);
	dumpEvents();
	
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
	if(!scrField->ref) return;

	// get script
	script* scr = scrField->ref->data;

	// start hook
	if(!evaluateFuncFromScript(scr->ctx, UPDATE_HOOK)) {
		logEvent(ERROR, EXEC, "Couldn't start entity \"%s\"", ent->name);
		dumpEvents();
	}
}

void scriptsUpdate(entity* ent) {
	// update entity
	scriptUpdate(ent);
	dumpEvents();
	
	// update children
	entity* cur = ent->child;
	while(cur) {
		scriptsUpdate(cur);
		cur = cur->peer;
	}
}
