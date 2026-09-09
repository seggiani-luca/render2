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

	// recurse for functions
	if(new->type == VAL_FUNCTION) {
		new->func.params = copyValue(orig->func.params);
		new->func.body = copyValue(orig->func.body);
	}

	return new;
}

// -- parsing

// produces a nil value
value* parseNil() {
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
	if(consume(buf, ")")) return parseNil(); 

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
	*tail = parseNil();

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
	
	// free functions 
	if(val->type == VAL_FUNCTION) {
		freeValue(val->func.params);
		freeValue(val->func.body);
	}

	// free this
	free(val);
}

// -- scripts

script* parseScript(char** buf) {
	script* scr = malloc(sizeof(script));
	if(!scr) return NULL;

	// consider a script as a list
	value* head = NULL;
    value** tail = &head;
	
	// until end of list 
	for(;;) {
		// get value
		value* val = parseValue(buf);
		if(!val) break; 

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
	}

	// terminate list
	*tail = parseNil();

	// assign
	scr->root = head;

	return scr;
}

void freeScript(script* scr) {
	freeValue(scr->root);
	free(scr);
}

// -- printing

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

	// dot notation
	if(head->type != VAL_NIL) {
		printf(" . ");
		printValue(head);
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
		case VAL_SYMBOL:   printSymbol(val);   break;
		case VAL_NUMBER:   printNumber(val);   break;
		case VAL_BOOL:     printBool(val);     break;
		case VAL_STRING:   printString(val);   break;
		case VAL_CONS:     printList(val);     break;
		case VAL_FUNCTION: printFunction(val); break;
		case VAL_NATIVE:   printNative(val);   break;
		case VAL_NIL:  	   printNil(val);      break;
		default: break;
	}
}

void printScript(script* scr) {
	// go through all expressions
	value *head = scr->root;
	while(head->type == VAL_CONS) {
		printValue(head->cons.car);
		printf("\n");
	
		// advance
		head = head->cons.cdr;
	}
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
void evaluateScript(environment* env, script* scr);
static envFrame nativeFrame;
void initNativeEnvironment();

environment* initEnvironment(script* scr) {
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
	evaluateScript(env, scr);

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
		printf("Frame owner: %p, Owns values: %d\n",
			head->frame->owner,
			head->frame->valuesOwned
		);
		printFrame(head->frame);
		if(head->next) printf("\n");

		head = head->next;
	}
}

// -- natives

// produces a nil value
value* arenaNil(environment* env) {
	value* nil = arenaAlloc(&env->arena, sizeof(value));
	nil->type = VAL_NIL;

	return nil;
}

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
	return arenaNil(env);
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
	func->func.body = body->cons.car;

	return func;
}

// definition of native begin function
value* nativeBegin(environment* env, value* args) {
	// keep track of last result
	value* result = arenaNil(env);

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
	int fls = condition->type == VAL_BOOL && !condition->boolean;
	if(!fls) return evaluateValue(env, branch1);
	else     return evaluateValue(env, branch2);
}

// definition of native quote function
value* nativeQuote(environment* env, value* args) {
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

	// create pair
	value* cons = arenaAlloc(&env->arena, sizeof(value));
	cons->type = VAL_CONS;
	cons->cons.car = first; 
	cons->cons.cdr = second; 

	return cons;
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

	// special forms
	{ .key = "define",   .fn = nativeDefine },
	{ .key = "lambda",   .fn = nativeLambda },
	{ .key = "begin",    .fn = nativeBegin  },
	{ .key = "if",       .fn = nativeIf     },
	{ .key = "quote",    .fn = nativeQuote  },
	{ .key = "cons",     .fn = nativeCons   },
	{ .key = "car",      .fn = nativeCar    },
	{ .key = "cdr",      .fn = nativeCdr    }
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
value* evaluateNumber(environment* env, value* val) {
	return val;
}

// evaluates a boolean 
value* evaluateBool(environment* env, value* val) {
	return val;
}

// evaluates a string 
value* evaluateString(environment* env, value* val) {
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
		value *arg = evaluateValue(env, args->cons.car);
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

void evaluateScript(environment* env, script* scr) {
	// go through all expressions
	value *head = scr->root;
	while(head->type == VAL_CONS) {
		evaluateValue(env, head->cons.car);
	
		// advance
		head = head->cons.cdr;
	}
}
