#ifndef SCRIPT_EXECUTE_H
#define SCRIPT_EXECUTE_H

#include "lisp.h"

// -- values

// creates a new symbol value
value* makeSymbol(arena* a, const char* sym);

// variant of makeSymbol which takes string length
value* makeSymbolN(arena* a, const char* sym, size_t len);

// creates a new number value
value* makeNumber(arena* a, float val);

// creates a new boolean value
value* makeBool(arena* a, int val);

// creates a new string value
value* makeString(arena* a, const char* str);

// creates a new list value
value* makeCons(arena* a, value* car, value* cdr);

// creates a new function value
value* makeFunction(arena* a, function* func);

// creates a new native function value
value* makeNative(arena* a, nativeFn fn);

// creates a new nil value
value* makeNil();

// -- sugar

// sugars a begin block from a statement list
value* makeBegin(arena* a, value* list);

// -- evaluating

// helper that checks if a value is true
int isTrue(value* val);

// applies a function to an argument list
value* applyFunction(arena* a, environment* env, function func, value* args);

// evaluates a symbol
value* evaluateSymbol(
	arena* a __attribute__ ((unused)),
	environment* env,
	value* val
);

// evaluates a number
value* evaluateNumber(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
);

// evaluates a boolean
value* evaluateBool(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
);

// evaluates a string
value* evaluateString(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
);

// evaluates a list
value* evaluateList(arena* a, environment* env, value* val);

// evaluates a function
value* evaluateFunction(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
);

// evaluates a native function
value* evaluateNative(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
);

// evaluates a nil value
value* evaluateNil(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
);

// evaluates a value
value* evaluateValue(arena* a, environment* env, value* val);


#endif
