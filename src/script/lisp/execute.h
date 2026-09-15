#ifndef SCRIPT_EXECUTE_H
#define SCRIPT_EXECUTE_H

#include "lisp.h"

// -- sugar

// produces a native symbol value
value* makeSymbol(value* nu, const char* sym);

// produces a CONS value
value* makeCons(value* nu, value* car, value* cdr);
		
// produces a bool value
value* makeBool(value* nu, int val);

// transforms a list of expressions into a begin block
value* makeBegin(value* nu, value* begin, value* list);

// produces a nil value
value* makeNil(value* nu);

// -- evaluating

// helper that checks if a value is true
int isTrue(value* val);

// evaluates a symbol 
value* evaluateSymbol(environment* env, value* val);

// evaluates a number
value* evaluateNumber(environment* env __attribute__((unused)), value* val);

// evaluates a boolean 
value* evaluateBool(environment* env __attribute__((unused)), value* val);

// evaluates a string 
value* evaluateString(environment* env __attribute__((unused)), value* val);

// applies a function
value* applyFunction(environment* env, function func, value* args);

// evaluates a list 
value* evaluateList(environment* env, value* val);

// evaluates a function 
value* evaluateFunction(environment* env __attribute__((unused)), value* val);

// evaluates a native 
value* evaluateNative(environment* env __attribute__((unused)), value* val);

// evaluates nil 
value* evaluateNil(environment* env __attribute__((unused)), value* val);

// actually evaluates value
value* doEvaluateValue(environment* env, value* val);

#endif
