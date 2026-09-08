#include "script.h"
#include "../parse/parse.h"
#include "arena/arena.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// -- values

// copies a value
value* copyValue(value* orig) {
	// allocate copy
	value* new = malloc(sizeof(value));
	if(!new) return NULL;

	// copy over
	memcpy(new, orig, sizeof(value));

	// recurse for CONS
	if(new->type == VAL_CONS) {
		new->cons.car = copyValue(orig->cons.car);
		new->cons.cdr = copyValue(orig->cons.cdr);
	}

	return new;
}

// copies a value, allocating it to an environment arena
value* copyValueArena(arena* a, value* orig) {
	// allocate copy
	value* new = arenaAlloc(a, sizeof(value));
	if(!new) return NULL;

	// copy over
	memcpy(new, orig, sizeof(value));

	// recurse for CONS
	if(new->type == VAL_CONS) {
		new->cons.car = copyValueArena(a, orig->cons.car);
		new->cons.cdr = copyValueArena(a, orig->cons.cdr);
	}

	return new;
}

// -- parsing

// produces a nil value
value* makeNil() {
	// allocate value 
	value* new = malloc(sizeof(value));
	if(!new) return NULL;
	
	// setup value 
	new->type = VAL_NIL;

	return new;
}

// foward declarations for list parsing
value* parseValue(char** buf);
void freeValue(value* val);

// parses a list value 
value* parseList(char** buf) {
	// begin list
	expect(buf, "(");
	eatWhitespace(buf);
	
	// check for empty lists (nils)
	if(consume(buf, ")")) return makeNil(); 

	value* head = NULL;
    value** tail = &head;
	
	// until end of list 
	for(;;) {
		// get value
		value* val = parseValue(buf);
		if(!val) {
			printf("Malformed CONS near %.20s\n", *buf);
			exit(1);
		}

		// allocate cell
        value *cell = malloc(sizeof(value));
        if(!cell) {
			freeValue(val);
			freeValue(head);
			return NULL;
		}

		// setup cell
		cell->type = VAL_CONS;
		cell->cons.car = val;
		cell->cons.cdr = NULL;

		// append cell 
        *tail = cell;
        tail = &cell->cons.cdr;

		// check for last value 
		eatWhitespace(buf);
		if(consume(buf, ")")) break;
	}

	// terminate list
	*tail = makeNil();

	return head;
}

// parses a number value 
value* parseNumber(char** buf) {
	// allocate value 
	value* new = malloc(sizeof(value));
	if(!new) return NULL;
	
	// setup value 
	new->type = VAL_NUMBER;

	// get number
	char *end;
	new->number = strtof(*buf, &end);

	// validate trash reads
	if (end == *buf) {
		printf("Invalid number in script near %.20s\n", *buf);
		exit(1);
	}
	
	// advance 
	*buf = end;

	return new;
}

// parses a symbol value 
value* parseSymbol(char** buf) {
	// allocate value 
	value* new = malloc(sizeof(value));
	if(!new) return NULL;
	
	// setup value 
	new->type = VAL_SYMBOL;

	// get symbol 
	char* start = *buf;
	while(**buf &&
	   !isWhitespace(**buf) &&
	   **buf != '(' &&
	   **buf != ')') {
		(*buf)++;
	}

	// get length 
	size_t len = *buf - start;
	if(len >= SYM_SIZE) len = SYM_SIZE - 1;

	// copy symbol over
	memcpy(new->symbol, start, len);
	new->symbol[len] = '\0';

	return new;
}

// walks to the end of a comment
void eatComment(char** buf) {
	while(**buf && **buf != '\n') (*buf)++;
    if(**buf == '\n') (*buf)++;
}

value* parseValue(char** buf) {
	eatWhitespace(buf);

	// eat comments if present
	while(**buf && **buf == ';') {
		eatComment(buf);
		eatWhitespace(buf);
	}

	// distinguish value type
	switch(**buf) {
		case '(': return parseList(buf);
		case '\0': return NULL;
		default:
			if(isDigit(**buf)) return parseNumber(buf);
			else               return parseSymbol(buf);
	}
}

void freeValue(value* val) {
	if(!val) return;

	// free CONS CAR and CDR 
	if(val->type == VAL_CONS) {
		freeValue(val->cons.car);
		freeValue(val->cons.cdr);
	}

	// free this
	free(val);
}

// -- scripts

script parseScript(char** buf) {
	script scr = NULL;
	expression** tail = &scr;	

	// fetch all expressions
	for(;;) {
		// get expression
		value* val = parseValue(buf);
		if(!val) return scr;
	
		// concatenate expression
		expression* expr = malloc(sizeof(expression));
		if(!expr) return scr;
		expr->next = NULL;
		expr->val = val;

		// advance
		*tail = expr;
		tail = &expr->next;
	}
}

void freeScript(script scr) {
	// go through all expressions
	expression* head = scr;
	while(head) {
		// advance first
		expression* next = head->next;

		freeValue(head->val);
		free(head);
		
		head = next;
	}
}

// -- printing

// prints a number value
void printNumber(value* val) {
	printf("%f", val->number);
}

// prints a symbol value
void printSymbol(value* val) {
	printf("%s", val->symbol);
}

// forward declaration for list printing
void printValue(value* val);

// prints a list value
void printList(value* val) {
	printf("(");

	// walk list
	value *head = val;
	while(head->type == VAL_CONS) {
		printValue(head->cons.car);
	
		// advance
		head = head->cons.cdr;
		if(head->type == VAL_CONS) printf(" ");
	}

	printf(")");
}

// prints a function value
void printFunction(value* val) {
	printf("(lambda ");

	// print args
	printList(val->func.params);

	printf(" ");
	
	// print body
	printList(val->func.body);
	
	printf(")");
}

// prints a native value 
void printNative(value* val) {
	printf("(Native)");
}

// prints a nil value
void printNil(value* val) {
	printf("()");
}

void printValue(value* val) {
	// distinguish value type
	switch(val->type) {
		case VAL_NUMBER:   printNumber(val);   break;
		case VAL_SYMBOL:   printSymbol(val);   break;
		case VAL_CONS:     printList(val);     break;
		case VAL_FUNCTION: printFunction(val); break;
		case VAL_NATIVE:   printNative(val);   break;
		case VAL_NIL:  	   printNil(val);      break;
		default: break;
	}
}

void printScript(script scr) {
	// go through all expressions
	expression* head = scr;
	while(head) {
		printValue(head->val);
		printf("\n");

		head = head->next;
	}
}

void printEnvironment(environment* env) {
	// go through all entries
	envEntry* head = env->root;
	while(head) {
		printf("%s: ", head->key);
		printValue(head->value);
		printf("\n");

		head = head->next;
	}
}

// -- environments

environment* newEnvironment() {
	environment* new = malloc(sizeof(environment));
	new->root = NULL;

	// initialize arena
	new->arena = newArena(ARENA_SIZ);

	return new;
}


void freeEnvironment(environment* env) {
	// go through all entries
	envEntry* head = env->root;
	while(head) {
		// advance first
		envEntry* next = head->next;

		// free only if owned
		if(head->value->owner == env) freeValue(head->value);

		// free entry
		free(head);

		head = next;
	}

	// free arena
	freeArena(&env->arena);

	free(env);
}

// appends entries to an environment 
void concatenateEnvironment(environment* env, envEntry* other) {
	// walk entries adding
	while(other) {
		addEnv(env, other->key, other->value);
		other = other->next;
	}
}

// finds an entry of any type in an environment
envEntry* queryEnv(environment* env, const char* key) {
	// go through all entries
	envEntry* head = env->root;
	while(head) {
		if(strcmp(head->key, key) == 0) return head;

		head = head->next;
	}

	// not found 
	return NULL;

}

void addEnv(environment* env, const char* key, value* val) {
	// go through all entries first
    envEntry** tail = &env->root;
	while(*tail) {
		envEntry* entry = *tail;

		if(strcmp(entry->key, key) == 0) {
			// copy over new value
			value* copy = copyValue(val);
			if(!copy) return;
			copy->owner = env;

			// free old value
			freeValue(entry->value);

			// set copy
			entry->value = copy;

			return;
		} 

		tail = &entry->next;
	}

	// create new entry 
	envEntry* new = malloc(sizeof(envEntry));
	if(!new) return;
	
	// setup entry
	new->next = NULL;

	// copy key over
	int len = strlen(key);
	if(len >= SYM_SIZE) len = SYM_SIZE - 1;
	strncpy(new->key, key, len);
	new->key[len] = '\0';
			
	// copy over new value
	new->value = copyValue(val);
	if(!new->value) {
		free(new);
		return;
	}
	new->value->owner = env;

	// append entry
	*tail = new; 
}

// -- natives

// typedef for arithmetic functions
typedef float (*arithmFunc)(float a, float b);

// definition of native arithmetic function
value* nativeArithm(environment* env, value* args, arithmFunc func) {
	// get arguments
	value* lhs = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	if(lhs->type != VAL_NUMBER || args->type != VAL_CONS) {
		printf("Invalid lhs arguments to +\n");
		exit(1);
	}

	value* rhs = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	if(rhs->type != VAL_NUMBER || args->type != VAL_NIL) {
		printf("Invalid rhs arguments to +\n");
		exit(1);
	}

	// allocate result
	value* result = arenaAlloc(&env->arena, sizeof(value));
	result->type = VAL_NUMBER;
	result->number = func(lhs->number, rhs->number);

	return result;
}

// definition of native add function
float add(float a, float b) { return a + b; }
value* nativeAdd(environment* env, value* args) {
	return nativeArithm(env, args, add);
}

// definition of native sub function
float sub(float a, float b) { return a - b; }
value* nativeSub(environment* env, value* args) {
	return nativeArithm(env, args, sub);
}

// definition of native mul function
float mul(float a, float b) { return a * b; }
value* nativeMul(environment* env, value* args) {
	return nativeArithm(env, args, mul);
}

// definition of native div function
float divs(float a, float b) { return a / b; }
value* nativeDiv(environment* env, value* args) {
	return nativeArithm(env, args, divs);
}

// definition of native define function
value* nativeDefine(environment* env, value* args) {
	// get and validate key
	value* key = args->cons.car;	
	if(key->type != VAL_SYMBOL) {
		printf("Trying to define a non-symbolic key\n");
		exit(1);
	}

	// get value
	if(args->cons.cdr->cons.cdr->type != VAL_NIL) {
		printf("Too many list elements for definition\n");
		exit(1);
	}
	value* val = evaluateValue(env, args->cons.cdr->cons.car);

	// define
	addEnv(env, key->symbol, val);

	// return nil
	value* nil = arenaAlloc(&env->arena, sizeof(value));
	nil->type = VAL_NIL;
	
	return nil;
}

// definition of native lambda function
value* nativeLambda(environment* env, value* args) {
	// get parameter list
	value* params = args->cons.car;

	// validate parameters
	value* head = params;
	while(head->type != VAL_NIL) {
		if(head->type != VAL_CONS) {
			printf("Malformed lambda parameter list\n");
			exit(1);
		}

		// make sure it's symbol
		value* param = head->cons.car;
		if(param->type != VAL_SYMBOL) {
			printf("Lambda parameter is not a symbol\n");
			exit(1);
		}

		head = head->cons.cdr;
	}

	// body starts after parameter list
	value* body = args->cons.cdr;
	if(body->type == VAL_NIL) {
		printf("Lambda requires a body\n");
		exit(1);
	}

	// allocate function
	value* func = arenaAlloc(&env->arena, sizeof(value));
	func->type = VAL_FUNCTION;

	// setup function
	func->func.params = params;
	func->func.body = body->cons.car;

	return func;
}

// default environment
static envEntry nativeEnv[] = {
	{
        .key    = "+",
		.value  = &(value){
			.type = VAL_NATIVE,
			.native = nativeAdd 
		},
        .next   = &nativeEnv[1]
    },
	{
        .key    = "-",
		.value  = &(value){
			.type = VAL_NATIVE,
			.native = nativeSub 
		},
        .next   = &nativeEnv[2]
    },
	{
        .key    = "*",
		.value  = &(value){
			.type = VAL_NATIVE,
			.native = nativeMul 
		},
        .next   = &nativeEnv[3]
    },	
	{
        .key    = "/",
		.value  = &(value){
			.type = VAL_NATIVE,
			.native = nativeDiv 
		},
        .next   = &nativeEnv[4]
    },
    {
        .key    = "define",
		.value  = &(value){
			.type = VAL_NATIVE,
			.native = nativeDefine
		},
        .next   = &nativeEnv[5]
    },
    {
        .key    = "lambda",
		.value  = &(value){
			.type = VAL_NATIVE,
			.native = nativeLambda 
		},
        .next   = NULL 
    },
};

// -- environment evaluating

// forward declaration for initEnvironment
void evaluateScript(environment* env, script scr);

environment* initEnvironment(script scr) {
	// allocate environment
	environment* env = newEnvironment();
	if(!env) return NULL;

	// initialize natives 
	concatenateEnvironment(env, nativeEnv);

	// evaluate script to environment
	evaluateScript(env, scr);

	return env;
}

// -- evaluating


// evaluates a number
value* evaluateNumber(environment* env, value* val) {
	return val;
}

// evaluates a symbol 
value* evaluateSymbol(environment* env, value* val) {
	envEntry* entry = queryEnv(env, val->symbol);
	if(entry) return entry->value;

	printf("Symbol \"%s\" unknown\n", val->symbol);
	exit(1);
}

// applies a function
value* applyFunction(environment* env, function func, value* args) {
	// get parameters to bind
	value* params = func.params;

	// local environment
	environment* local = newEnvironment(); 
	concatenateEnvironment(local, env->root);

	// walk function params alongside cdr
	while(params && params->type != VAL_NIL
	&&    args   && args->type   != VAL_NIL) {
		// bind parameter
		value *arg = evaluateValue(env, args->cons.car);
		addEnv(local, params->cons.car->symbol, arg);

		// advance
		params = params->cons.cdr;
		args   = args->cons.cdr;
	}

	// check sizes match
	if(params->type != args->type) {
		printf("Wrong number of arguments\n");
		freeEnvironment(local);
		exit(1);
	}

	// apply function
	value* result = evaluateValue(local, func.body);
	
	// copy to arena
	value* copy = copyValueArena(&env->arena, result);

	// cleanup
	freeEnvironment(local);

	return copy;
}

// evaluates a list 
value* evaluateList(environment* env, value* val) {
	// get function symbol
	value* car = val->cons.car;

	// evaluate function symbol
	value* func = evaluateValue(env, car);
	
	// get arguments
	value* args = val->cons.cdr;

	// distinguish on callable types
	switch(func->type) {
		case VAL_FUNCTION: return applyFunction(env, func->func, args);
		case VAL_NATIVE:   return func->native(env, args);
		default: break;
	}

	printf("Trying to call non-callable value\n");
	exit(1);
}

// evaluates a function 
value* evaluateFunction(environment* env, value* val) {
	return val;
}

// evaluates a native 
value* evaluateNative(environment* env, value* val) {
	return val;
}

// evaluates nil 
value* evaluateNil(environment* env, value* val) {
	return val;
}

value* evaluateValue(environment* env, value* val) {
	// distinguish value type
	switch(val->type) {
		case VAL_NUMBER:   return evaluateNumber(env, val);  
		case VAL_SYMBOL:   return evaluateSymbol(env, val);  
		case VAL_CONS:     return evaluateList(env, val);    
		case VAL_FUNCTION: return evaluateFunction(env, val);
		case VAL_NATIVE:   return evaluateNative(env, val);
		case VAL_NIL:      return evaluateNil(env, val);     
		default: return NULL;
	}
}

void evaluateScript(environment* env, script scr) {
	// go through all expressions
	expression* head = scr;
	while(head) {
		// evaluate expression's value
		evaluateValue(env, head->val);

		head = head->next;
	}
}
