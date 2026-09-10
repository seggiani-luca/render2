#include "lisp.h"
#include "../../parse/parse.h"
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

	// recurse for functions
	if(new->type == VAL_FUNCTION) {
		new->func.params = copyValue(orig->func.params);
		new->func.body = copyValue(orig->func.body);
	}

	return new;
}

// gets the length of a CONS list
size_t consLen(value *v) {
    size_t len = 0;

	// go through whole list
    while (v && v->type == VAL_CONS) {
        len++;
        v = v->cons.cdr;
    }

    return len;
}

// -- sugar

// produces a native value
value* makeSymbol(value* new, const char* sym) {
	new->type = VAL_SYMBOL;
	strcpy(new->symbol, sym);

	return new;
}

// produces a CONS value
value* makeCons(value* new, value* car, value* cdr) {
	new->type = VAL_CONS;
	new->cons.car = car;
	new->cons.cdr = cdr;

	return new;
}

// produces a bool value
value* makeBool(value* new, int val) {
	new->type = VAL_BOOL;
	new->boolean = val;

	return new;
}

// transforms a list of expressions into a begin block
value* makeBegin(value* new, value* begin, value* list) {
	if(!new || !begin) return NULL;
	
	// setup value 
	new->type = VAL_CONS;
	new->cons.cdr = list;

	// setup native begin
	begin = makeSymbol(begin, "begin");

	// attach native begin
	new->cons.car = begin;

	return new;
}

// produces a nil value
value* makeNil(value* new) {
	if(!new) return NULL;
	
	// setup value 
	new->type = VAL_NIL;

	return new;
}

// -- parsing

// foward declarations for list parsing
value* parseValue(char** buf);
void freeValue(value* val);

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

// parses a list value 
value* parseList(char** buf) {
	// begin list
	expect(buf, "(");
	eatWhitespace(buf);
	
	// check for empty lists (nils)
	if(consume(buf, ")")) return makeNil(malloc(sizeof(value)));

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
        value* cell = malloc(sizeof(value));
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
	*tail = makeNil(malloc(sizeof(value)));

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

// parses a boolean value 
value* parseBool(char** buf) {
	// allocate value 
	value* new = malloc(sizeof(value));
	if(!new) return NULL;
	
	// setup value 
	new->type = VAL_BOOL;

	// get boolean
	if(consume(buf, "#t")) {
		new->boolean = 1;
	} else if(consume(buf, "#f")) {
		new->boolean = 0;
	} else {
		printf("Invalid boolean value near %.20s\n", *buf);
		exit(1);
	}

	return new;
}

// parses a string value
value* parseString(char** buf) {
	// allocate value 
	value* new = malloc(sizeof(value));
	if(!new) return NULL;
	
	// setup value 
	new->type = VAL_STRING;

	// get string
	const char* str = readString(buf);
	new->string = str;

	return new;
}

// walks to the end of a comment
void eatComment(char** buf) {
	while(**buf && **buf != '\n') (*buf)++;
    if(**buf == '\n') (*buf)++;
}

// parses a value
value* parseValue(char** buf) {
	eatWhitespace(buf);

	// eat comments if present
	while(**buf && **buf == ';') {
		eatComment(buf);
		eatWhitespace(buf);
	}

	// distinguish value type
	switch(**buf) {
		case '#':              return parseBool(buf);
		case '"':              return parseString(buf); 
		case '(':              return parseList(buf);
		case '\0':             return NULL;
		default:
			if(isDigit(*buf))  return parseNumber(buf);
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
	
	// free functions 
	if(val->type == VAL_FUNCTION) {
		freeValue(val->func.params);
		freeValue(val->func.body);
	}

	// free this
	free(val);
}

// -- scripts

value* parseScript(char** buf) {
	// consider a script as a list
	value* head = NULL;
    value** tail = &head;
	
	// until end of list 
	for(;;) {
		// get value
		value* val = parseValue(buf);
		if(!val) break; 

		// allocate cell
        value* cell = malloc(sizeof(value));
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
	}

	// terminate list
	*tail = makeNil(malloc(sizeof(value)));

	// sugar begin
	value* begin = makeBegin(
		malloc(sizeof(value)), 
		malloc(sizeof(value)),
		head);

	return begin;
}

// -- printing

// helper for depth increment
#define INC_DEPTH (depth < 0 ? depth : depth + 1)

// inserts tabs to reach given depth
void tabulate(int depth) {
	for(int i = 0; i < depth; i++) printf("  ");
}

// prints a symbol value
void printSymbol(value* val) {
	printf("%s", val->symbol);
}

// prints a number value
void printNumber(value* val) {
	printf("%f", val->number);
}

void printBool(value* val) {
	printf("%s", val->boolean ? "#t" : "#f");
}

// prints a string value
void printString(value* val) {
	printf("\"%s\"", val->string);
}

// forward declaration for list printing
void doPrintValue(value* val, int depth);

// prints a list value
void printList(value* val, int depth) {
	printf("(");

	// should print vertically?
	int vertical = 0;

	// walk list
	value* head = val;
	while(head->type == VAL_CONS) {
	
		// print value
		doPrintValue(head->cons.car, INC_DEPTH);
	
		// advance
		head = head->cons.cdr;

		// pretty-print
		if(head->type == VAL_CONS) {
			// see if next argument is atomic
			value* car = head->cons.car;
			int atomic = !(car->type == VAL_CONS && consLen(car) > PP_CONS_LEN)
			&&           car->type != VAL_FUNCTION;

			// try to stay on one line
			if(depth < 0 || (atomic && !vertical)) printf(" ");
			else {
				// otherwise begin vertical
				vertical = 1;
				printf("\n");
				tabulate(depth + 1);
			}
		}
	}

	// dot notation
	if(head->type != VAL_NIL) {
		printf(" . ");
		doPrintValue(head, depth);
	}

	printf(")");
}

// prints a function value
void printFunction(value* val, int depth) {
	printf("(lambda ");

	// print args
	printList(val->func.params, INC_DEPTH);

	printf(" ");
	
	// print body
	printList(val->func.body, INC_DEPTH);
	
	printf(")");
}

// prints a native value 
void printNative(value* val __attribute__((unused))) {
	printf("(Native)");
}

// prints a nil value
void printNil(value* val __attribute__((unused))) {
	printf("()");
}

// actually prints a value, taking depth into account 
void doPrintValue(value* val, int depth) {
	// distinguish value type
	switch(val->type) {
		case VAL_SYMBOL:   printSymbol(val);          break;
		case VAL_NUMBER:   printNumber(val);          break;
		case VAL_BOOL:     printBool(val);            break;
		case VAL_STRING:   printString(val);          break;
		case VAL_CONS:     printList(val, depth);     break;
		case VAL_FUNCTION: printFunction(val, depth); break;
		case VAL_NATIVE:   printNative(val);          break;
		case VAL_NIL:  	   printNil(val);             break;
		default: break;
	}
}

void printValue(value* val) {
	doPrintValue(val, -1);
}

void prettyPrintValue(value* val) {
	doPrintValue(val, 0);
}

// -- environment frames

envFrame* newFrame(void* owner, int valuesOwned) {
	// allocate frame
	envFrame* frame = malloc(sizeof(envFrame));
	if(!frame) return NULL;

	// setup frame
	frame->root = NULL;
	frame->owner = owner;
	frame->valuesOwned = valuesOwned;

	return frame;
}

// frees an environment frame
void freeFrame(envFrame* frame) {
	if(!frame) return;

	// go through all entries
	envEntry* head = frame->root;
	while(head) {
		// advance first
		envEntry* next = head->next;

		// free only if owned
		if(frame->valuesOwned) freeValue(head->value);
		free(head);

		head = next;
	}

	free(frame);
}

envEntry* queryFrame(envFrame* frame, const char* key) {
	// go through all entries
	envEntry* head = frame->root;
	while(head) {
		if(strcmp(head->key, key) == 0) return head;

		head = head->next;
	}

	// not found 
	return NULL;

}

void addToFrame(envFrame* frame, const char* key, value* val) {
	// if should own, make a copy 
	if(frame->valuesOwned) val = copyValue(val);

	// go through all entries first
    envEntry** tail = &frame->root;
	while(*tail) {
		envEntry* entry = *tail;

		if(strcmp(entry->key, key) == 0) {
			// entry already exists
	
			// if owned, free old value 
			if(frame->valuesOwned) freeValue(entry->value);
			
			// assign value
			entry->value = val;

			return;
		} 

		tail = &entry->next;
	}
	// entry doesn't exist 

	// create new entry 
	envEntry* new = malloc(sizeof(envEntry));
	if(!new) {
		if(frame->valuesOwned) freeValue(val);
		return;
	} 
	
	// setup entry
	new->next = NULL;

	// copy key over
	int len = strlen(key);
	if(len >= SYM_SIZE) len = SYM_SIZE - 1;
	strncpy(new->key, key, len);
	new->key[len] = '\0';
			
	// assign value
	new->value = val;

	// append entry
	*tail = new; 
}

void printFrame(envFrame* frame) {
	// go through all entries
	envEntry* head = frame->root;
	while(head) {
		printf("%s:\t", head->key);
		printValue(head->value);
		printf("\n");

		head = head->next;
	}
}

// -- environments

void pushFrame(environment* env, envFrame* frame) {
	// allocate link
	envLink* link = malloc(sizeof(envLink));
	if(!link) return;
    
	// setup link
	link->frame = frame;

	// insert at root
    link->next = env->root;
	env->root = link;
}

envFrame* popFrame(environment* env) {
    if(!env->root) return NULL;

	// extract root 
    envLink* link = env->root;
    env->root = link->next;

	// free link
	envFrame* frame = link->frame;
	free(link);

    return frame;
}

envFrame* topFrame(environment* env) {
    if (!env->root) return NULL;

 	// return root frame
	return env->root->frame;
}

envEntry* queryEnvironment(environment* env, const char* key) {
	// go through all frames
	envLink* head = env->root;
	while(head) {
		envEntry* found = queryFrame(head->frame, key);
		if(found) return found;

		head = head->next;
	}

	// not found
	return NULL;
}

// get a new environment
environment* newEnvironment() {
	// allocate environment
	environment* env = malloc(sizeof(environment));
	if(!env) return NULL;

	// setup environment
	env->root = NULL;

	// initialize arena
	env->arena = newArena(ARENA_SIZ);

	return env;
}

// forward declarations for initEnvironment
static envFrame nativeFrame;
void initNativeEnvironment();

environment* initEnvironment(value* scr) {
	// allocate environment
	environment* env = newEnvironment();
	if(!env) return NULL;

	// push native frame
	if(nativeFrame.root == NULL) initNativeEnvironment();
	pushFrame(env, &nativeFrame);
	
	// push script frame
	env->scriptFrame = newFrame(
		env, // owned by this environment
		1    // owns values
	);
	if(!env->scriptFrame) {
		freeArena(&env->arena);
		free(env);
		return NULL;
	}
	pushFrame(env, env->scriptFrame);

	// evaluate script to environment
	evaluateValue(env, scr);

	return env;
}

void freeEnvironment(environment* env) {
	if(!env) return;

	// go through all frames 
	envLink* head = env->root;
	while(head) {
		// advance first
		envLink* next = head->next;

		// free only if owned
		if(head->frame->owner == env) freeFrame(head->frame);
		free(head);

		head = next;
	}

	// free arena
	freeArena(&env->arena);

	free(env);
}

void printEnvironment(environment* env) {
	// go through all frames
	envLink* head = env->root;
	while(head) {
		printf("Frame owner: %p,\tOwns values: %d\n",
			head->frame->owner,
			head->frame->valuesOwned
		);
		printFrame(head->frame);
		if(head->next) printf("\n");

		head = head->next;
	}
}

// -- natives

// typedef for arithmetic functions
typedef float (*arithmFunc)(float a, float b);

// definition of native arithmetic function
value* nativeArithm(environment* env, value* args, arithmFunc func) {
	if(args->type != VAL_CONS) {
		printf("Arithmetic op. requires first argument\n");
		exit(1);
	}

	// get arguments
	value* lhs = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	if(lhs->type != VAL_NUMBER || args->type != VAL_CONS) {
		printf("Invalid lhs arguments to arithmetic op.\n");
		exit(1);
	}

	value* rhs = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	if(rhs->type != VAL_NUMBER || args->type != VAL_NIL) {
		printf("Invalid rhs arguments to arithmetic op.\n");
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

// typedef for relational functions
typedef int (*relateFunc)(float a, float b);

// definition of native relational function
value* nativeRelate(environment* env, value* args, relateFunc func) {
	if(args->type != VAL_CONS) {
		printf("Relational op. requires first argument\n");
		exit(1);
	}
	
	// get arguments
	value* lhs = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	if(lhs->type != VAL_NUMBER || args->type != VAL_CONS) {
		printf("Invalid lhs arguments to relational op.\n");
		exit(1);
	}

	value* rhs = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	if(rhs->type != VAL_NUMBER || args->type != VAL_NIL) {
		printf("Invalid rhs arguments to relational op.\n");
		exit(1);
	}

	// allocate result
	value* result = arenaAlloc(&env->arena, sizeof(value));
	result->type = VAL_BOOL;
	result->boolean = func(lhs->number, rhs->number);

	return result;
}

// definition of native less function
int less(float a, float b) { return a < b; }
value* nativeLess(environment* env, value* args) {
	return nativeRelate(env, args, less);
}

// definition of native lessEqual function
int lessEqual(float a, float b) { return a <= b; }
value* nativeLessEqual(environment* env, value* args) {
	return nativeRelate(env, args, lessEqual);
}

// definition of native greater function
int greater(float a, float b) { return a > b; }
value* nativeGreater(environment* env, value* args) {
	return nativeRelate(env, args, greater);
}

// definition of native greaterEqual function
int greaterEqual(float a, float b) { return a >= b; }
value* nativeGreaterEqual(environment* env, value* args) {
	return nativeRelate(env, args, greaterEqual);
}

// definition of native equal function
int equal(float a, float b) { return a == b; }
value* nativeEqual(environment* env, value* args) {
	return nativeRelate(env, args, equal);
}

// definition of native notEqual function
int notEqual(float a, float b)  { return a != b; }
value* nativeNotEqual(environment* env, value* args) {
	return nativeRelate(env, args, notEqual);
}

// definition of native define function
value* nativeDefine(environment* env, value* args) {
	if(args->type != VAL_CONS) {
		printf("Define requires symbol to define\n");
		exit(1);
	}

	// get and validate key
	value* key = args->cons.car;
	args = args->cons.cdr;
	
	// desugar compact functions
	if(key->type == VAL_CONS) {
		value* params = key->cons.cdr;
		key = key->cons.car;
		value* body = args;

		// validate
		if(key->type != VAL_SYMBOL) {
			printf("Trying to define a desugared non-symbolic key\n");
			exit(1);
    	}

		// make lambda
		value* lambdaSym = makeSymbol(arenaAlloc(&env->arena, sizeof(value)),
			"lambda");
		value* lambdaArgs = makeCons(
			arenaAlloc(&env->arena, sizeof(value)),
			params,
			body
		);
		value* lambda = makeCons(
			arenaAlloc(&env->arena, sizeof(value)),
			lambdaSym,
			lambdaArgs	
		);
		
		// rebuild args
		args = makeCons(
			arenaAlloc(&env->arena, sizeof(value)),
			lambda,
			makeNil(arenaAlloc(&env->arena, sizeof(value)))
		);
	}

	if(key->type != VAL_SYMBOL) {
		printf("Trying to define a non-symbolic key\n");
		exit(1);
	}

	// get definition 
	if(args->type != VAL_CONS) {
		printf("Define requires symbol definition\n");
		exit(1);
	}
	if(args->cons.cdr->type != VAL_NIL) {
		printf("Define takes exactly two arguments\n");
		exit(1);
	}
	value* val = evaluateValue(env, args->cons.car);

	// define
	addToFrame(env->scriptFrame, key->symbol, val);

	// return nil	
	return makeNil(arenaAlloc(&env->arena, sizeof(value)));
}

// definition of native lambda function
value* nativeLambda(environment* env, value* args) {
	if(args->type != VAL_CONS) {
		printf("Lambda requires a parameter list\n");
		exit(1);
	}

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

	// sugar begin
	value* begin = makeBegin(
		arenaAlloc(&env->arena, sizeof(value)), 
		arenaAlloc(&env->arena, sizeof(value)),
		body);
	func->func.body = begin;

	return func;
}

// definition of native begin function
value* nativeBegin(environment* env, value* args) {
	// keep track of last result
	value* result = makeNil(arenaAlloc(&env->arena, sizeof(value)));

	// execute all found expressions
	while(args->type == VAL_CONS) {
		result = evaluateValue(env, args->cons.car);
		args = args->cons.cdr;
	}

	// make sure is end
	if(args->type != VAL_NIL) {
		printf("Malformed begin body\n");
		exit(1);
	}

	return result;
}

// definition of native quote function
value* nativeQuote(environment* env __attribute__((unused)), value* args) {
	if(args->type != VAL_CONS) {
		printf("Quote requires data\n");
		exit(1);
	}

	// get quoted data
	value* quote = args->cons.car;

	// exactly one argument
	if(args->cons.cdr->type != VAL_NIL) {
		printf("Quote takes exactly one argument\n");
		exit(1);
	}

	return quote;
}

// definition of native list function
value* nativeList(environment* env __attribute__((unused)), value* args) {
	if(args->type == VAL_NIL) return args;

	// go through arguments
	value* result = NULL;
	value** tail = &result;
	while(args->type == VAL_CONS) {
		// evaluate argument
		value* val = evaluateValue(env, args->cons.car);

		// concatenate to list
		value* node = makeCons(
			arenaAlloc(&env->arena, sizeof(value)),
			val,
			makeNil(arenaAlloc(&env->arena, sizeof(value)))
		);

		// append node
		*tail = node;
		tail = &node->cons.cdr;

		// advance
		args = args->cons.cdr;
	}

	if(args->type != VAL_NIL) {
		printf("Malformed list arguments\n");
		exit(1);
	}

	return result;
}

// definition of native cons function
value* nativeCons(environment* env, value* args) {
	// first value 
	if(args->type != VAL_CONS) {
		printf("Cons requires first argument\n");
		exit(1);
	}

	value* first = evaluateValue(env, args->cons.car); 
	args = args->cons.cdr;

	// second value 
	if(args->type != VAL_CONS) {
		printf("Cons requires second argument\n");
		exit(1);
	}

	value* second = evaluateValue(env, args->cons.car); 
	args = args->cons.cdr;

	// exactly two arguments
	if(args->type != VAL_NIL) {
		printf("Cons takes exactly two arguments\n");
		exit(1);
	}

	return makeCons(
		arenaAlloc(&env->arena, sizeof(value)),
		first,
		second
	); 
}

// definition of native car function
value* nativeCar(environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		printf("Car requires an argument\n");
		exit(1);
	}

	// take pair
	value* pair = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		printf("Car takes exactly one argument\n");
		exit(1);
	}
	if(pair->type != VAL_CONS) {
		printf("Car requires a pair\n");
		exit(1);
	}

	// car
	return pair->cons.car;
}

// definition of native cdr function
value* nativeCdr(environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		printf("Cdr requires an argument\n");
		exit(1);
	}

	// take pair
	value* pair = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		printf("Cdr takes exactly one argument\n");
		exit(1);
	}
	if(pair->type != VAL_CONS) {
		printf("Cdr requires a pair\n");
		exit(1);
	}

	// cdr
	return pair->cons.cdr;
}

// definition of native eval function
value* nativeEval(environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		printf("Eval requires an argument\n");
		exit(1);
	}

	// take value 
	value* value = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		printf("Eval takes exactly one argument\n");
		exit(1);
	}

	// actually evaluate
	return evaluateValue(env, value);
}

// helper that checks if a value is true
int isTrue(value* val) {
	int fls = val->type == VAL_BOOL && !val->boolean;
	return !fls;
}

// definition of native if function
value* nativeIf(environment* env, value* args) {
	if(args->type != VAL_CONS) {
		printf("If requires condition\n");
		exit(1);
	}

	// get condition first
	value* condition = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;

	// first branch
	if(args->type != VAL_CONS) {
		printf("If requires first branch\n");
		exit(1);
	}

	value* branch1 = args->cons.car;
	args = args->cons.cdr;

	// second branch
	if(args->type != VAL_CONS) {
		printf("If requires second branch\n");
		exit(1);
	}

	value* branch2 = args->cons.car;
	args = args->cons.cdr;

	// exactly three arguments
	if(args->type != VAL_NIL) {
		printf("If takes exactly three arguments\n");
		exit(1);
	}

	// execute appropiate branch
	if(isTrue(condition)) return evaluateValue(env, branch1);
	else                  return evaluateValue(env, branch2);
}

// definition of native and function
value* nativeAnd(environment* env, value* args) {
	// first value
	if(args->type != VAL_CONS) {
		printf("And requires first argument\n");
		exit(1);
	}

	value* first = args->cons.car; 
	args = args->cons.cdr;

	// second value
	if(args->type != VAL_CONS) {
		printf("And requires second argument\n");
		exit(1);
	}

	value* second = args->cons.car;
	args = args->cons.cdr;

	// exactly two arguments
	if(args->type != VAL_NIL) {
		printf("And takes exactly two arguments\n");
		exit(1);
	}

	// short circuit
	value* firstVal = evaluateValue(env, first);
	if(!isTrue(firstVal)) return firstVal;

	return evaluateValue(env, second);
}

// definition of native or function
value* nativeOr(environment* env, value* args) {
	// first value
	if(args->type != VAL_CONS) {
		printf("Or requires first argument\n");
		exit(1);
	}

	value* first = args->cons.car; 
	args = args->cons.cdr;

	// second value
	if(args->type != VAL_CONS) {
		printf("Or requires second argument\n");
		exit(1);
	}

	value* second = args->cons.car;
	args = args->cons.cdr;

	// exactly two arguments
	if(args->type != VAL_NIL) {
		printf("Or takes exactly two arguments\n");
		exit(1);
	}

	// short circuit
	value* firstVal = evaluateValue(env, first);
	if(isTrue(firstVal)) return firstVal;

	return evaluateValue(env, second);
}

// definition of native not function
value* nativeNot(environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		printf("Not requires an argument\n");
		exit(1);
	}

	// take value 
	value* val = evaluateValue(env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		printf("Not takes exactly one argument\n");
		exit(1);
	}

	// take the opposite
	return makeBool(
		arenaAlloc(&env->arena, sizeof(value)),
		!isTrue(val)
	); 
}

// helper struct for native environment
typedef struct {
	// key of entry
	char key[SYM_SIZE];

	// native function
	nativeFn fn;
} nativeEnvEntry;

// native environment entries helper
static nativeEnvEntry nativeEntriesHelper[] = {
	// arithmetic
	{ .key = "+",        .fn = nativeAdd },
	{ .key = "-",        .fn = nativeSub },
	{ .key = "*",        .fn = nativeMul },
	{ .key = "/",        .fn = nativeDiv },

	// relational
	{ .key = "<",        .fn = nativeLess         },
	{ .key = "<=",       .fn = nativeLessEqual    },
	{ .key = ">",        .fn = nativeGreater      },
	{ .key = ">=",       .fn = nativeGreaterEqual },
	{ .key = "=",        .fn = nativeEqual        },
	{ .key = "!=",       .fn = nativeNotEqual     },

	// environment
	{ .key = "define",   .fn = nativeDefine },

	// functions
	{ .key = "lambda",   .fn = nativeLambda },

	// sequential
	{ .key = "begin",    .fn = nativeBegin },

	// data manipulatoin
	{ .key = "quote",    .fn = nativeQuote },
	{ .key = "list",     .fn = nativeList  },
	{ .key = "cons",     .fn = nativeCons  },
	{ .key = "car",      .fn = nativeCar   },
	{ .key = "cdr",      .fn = nativeCdr   },
	{ .key = "eval",     .fn = nativeEval  },

	// conditionals
	{ .key = "if",       .fn = nativeIf  },
	{ .key = "and",      .fn = nativeAnd },
	{ .key = "or",       .fn = nativeOr  },
	{ .key = "not",      .fn = nativeNot }
};

// number of native entries
#define NATIVE_ENTRIES (sizeof(nativeEntriesHelper) / sizeof(nativeEnvEntry))

// native environment entries
static envEntry nativeEntries[NATIVE_ENTRIES];
static value    nativeValues[NATIVE_ENTRIES];

// native environment frame (lazy setup)
static envFrame nativeFrame = {
	.root = NULL,
	.valuesOwned = 0,
	.owner = NULL
};

// initializes the native environment
void initNativeEnvironment() {
	// go through all native entries
	for(size_t i = 0; i < NATIVE_ENTRIES; i++) {
		nativeEnvEntry* help  = &nativeEntriesHelper[i];
		envEntry*       entry = &nativeEntries[i];	
		value*          val   = &nativeValues[i];
	
		// initialize entry
		memcpy(entry->key, help->key, SYM_SIZE);
		entry->next = i == NATIVE_ENTRIES - 1 ? NULL : &nativeEntries[i + 1];

		// initialize value and assign
		val->type = VAL_NATIVE;
		val->native = help->fn;
		entry->value = val;
	}

	// set root
	nativeFrame.root = &nativeEntries[0];
}

// -- evaluating

// evaluates a symbol 
value* evaluateSymbol(environment* env, value* val) {
	envEntry* entry = queryEnvironment(env, val->symbol);
	if(entry) return entry->value;

	printf("Symbol \"%s\" unknown\n", val->symbol);
	exit(1);
}

// evaluates a number
value* evaluateNumber(environment* env __attribute__((unused)), value* val) {
	return val;
}

// evaluates a boolean 
value* evaluateBool(environment* env __attribute__((unused)), value* val) {
	return val;
}

// evaluates a string 
value* evaluateString(environment* env __attribute__((unused)), value* val) {
	return val;
}

// applies a function
value* applyFunction(environment* env, function func, value* args) {
	// get parameters to bind
	value* params = func.params;

	// local environment frame
	envFrame* local = newFrame(
		env, // owned by this environment
		0    // doesn't own values
	);
	if(!local) return NULL;

	// walk function params alongside cdr
	while(params && params->type != VAL_NIL
	&&    args   && args->type   != VAL_NIL) {
		// bind parameter
		value* arg = evaluateValue(env, args->cons.car);
		addToFrame(local, params->cons.car->symbol, arg);

		// advance
		params = params->cons.cdr;
		args   = args->cons.cdr;
	}

	// check sizes match
	if((params && params->type != VAL_NIL)
	|| (args   && args->type   != VAL_NIL)) {
		printf("Wrong number of arguments\n");
		freeFrame(local);
		exit(1);
	}

	// push local frame
	pushFrame(env, local);

	// apply function
	value* result = evaluateValue(env, func.body);
	
	// cleanup
	freeFrame(popFrame(env));

	return result; 
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
value* evaluateFunction(environment* env __attribute__((unused)), value* val) {
	return val;
}

// evaluates a native 
value* evaluateNative(environment* env __attribute__((unused)), value* val) {
	return val;
}

// evaluates nil 
value* evaluateNil(environment* env __attribute__((unused)), value* val) {
	return val;
}

value* evaluateValue(environment* env, value* val) {
	// distinguish value type
	switch(val->type) {
		case VAL_SYMBOL:   return evaluateSymbol(env, val);  
		case VAL_NUMBER:   return evaluateNumber(env, val);  
		case VAL_BOOL:     return evaluateBool(env, val);  
		case VAL_STRING:   return evaluateString(env, val);  
		case VAL_CONS:     return evaluateList(env, val);    
		case VAL_FUNCTION: return evaluateFunction(env, val);
		case VAL_NATIVE:   return evaluateNative(env, val);
		case VAL_NIL:      return evaluateNil(env, val);     
		default: return NULL;
	}
}
