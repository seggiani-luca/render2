#include "lisp.h"
#include "execute.h"
#include "../../parse/parse.h"
#include "../../exception/exception.h"
#include "../script.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// helper for heap / arena allocation
void* frAllocate(arena* a, size_t siz) {
	return a
		? arenaAlloc(a, siz)
		: xmalloc(siz);
}

// -- values

// copies a value to a the heap 
value* copyValue(value* orig) {
	// allocate copy
	value* nu = xmalloc(sizeof(value)); 

	// copy over
	memcpy(nu, orig, sizeof(value));

	// recurse for CONS
	if(nu->type == VAL_CONS) {
		nu->cons.car = copyValue(orig->cons.car);
		nu->cons.cdr = copyValue(orig->cons.cdr);
	}

	// recurse for functions
	if(nu->type == VAL_FUNCTION) {
		nu->func.params = copyValue(orig->func.params);
		nu->func.body   = copyValue(orig->func.body);
	}

	return nu;
}

// frees a value
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

// -- values

value* makeSymbol(arena* a, const char* sym) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_SYMBOL;

	// symbol string
	strncpy(nu->symbol, sym, SYM_SIZE);
	nu->symbol[SYM_SIZE - 1] = '\0';

	return nu;
}

value* makeSymbolN(arena* a, const char* sym, size_t len) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_SYMBOL;

	// symbol string
	if(len >= SYM_SIZE) len = SYM_SIZE - 1;
	memcpy(nu->symbol, sym, len);
	nu->symbol[len] = '\0';

	return nu;
}


value* makeNumber(arena* a, float val) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_NUMBER;
	nu->number = val;

	return nu;
}

value* makeBool(arena* a, int val) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_BOOL;
	nu->boolean = val;

	return nu;
}

value* makeString(arena* a, const char* str) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_STRING;
	nu->string = str;

	return nu;
}

value* makeCons(arena* a, value* car, value* cdr) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_CONS;
	nu->cons.car = car;
	nu->cons.cdr = cdr;

	return nu;
}

value* makeFunction(arena* a, function* func) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_FUNCTION;
	nu->func = *func; // by value

	return nu;
}

value* makeNative(arena* a, nativeFn fn) {
	value* nu = frAllocate(a, sizeof(value));
	nu->type = VAL_NATIVE;
	nu->native = fn; 

	return nu;
}

value* makeNil() {
	static value nil = (value){
		.type = VAL_NIL
	};

	return &nil; 
}

// -- sugar

value* makeBegin(arena* a, value* list) {
	value* begin = makeSymbol(a, "begin");

	// make list
	return makeCons(a, begin, list); 
}

// -- parsing

// walks to the end of a comment
void eatComment(char** buf) {
	while(**buf && **buf != '\n') (*buf)++;
	if(**buf == '\n') (*buf)++;
}

// eats multiple comments
void eatComments(char** buf) {
	// eat comments if present
	while(**buf && **buf == ';') {
		eatComment(buf);
		eatWhitespace(buf);
	}
}

// foward declarations for list parsing
value* parseValue(arena* a, char** buf);

// parses a symbol value 
value* parseSymbol(arena* a, char** buf) {
	if(!**buf
	|| isWhitespace(**buf)
	|| **buf == '('
	|| **buf == ')') {
		logEvent(ERROR, LISP, "Trying to parse invalid symbol");
		throw;
	}

	// get symbol 
	char* start = *buf;
	while(**buf
	&& !isWhitespace(**buf)
	&& **buf != '('
	&& **buf != ')') {
		(*buf)++;
	}

	// get length 
	size_t len = *buf - start;

	return makeSymbolN(a, start, len);
}

// parses a list value 
value* parseList(arena* a, char** buf) {
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
		value* val = parseValue(a, buf);
		if(!val) {
			logEvent(ERROR, LISP, "Malformed CONS near %.20s", *buf);
			throw;
		}

		// allocate cell
		value* cell = makeCons(a, val, NULL);

		// append cell 
		*tail = cell;
		tail = &cell->cons.cdr;

		// eat stuff
		eatWhitespace(buf);
		eatComments(buf);

		// check for last value 
		if(consume(buf, ")")) break;
	}

	// terminate list
	*tail = makeNil();

	return head;
}

// parses a number value 
value* parseNumber(arena* a, char** buf) {
	// get number
	char *end;
	float num = strtof(*buf, &end);

	// validate trash reads
	if (end == *buf) {
		logEvent(ERROR, LISP, "Invalid number in script near %.20s", *buf);
		throw;
	}

	// advance 
	*buf = end;

	return makeNumber(a, num); 
}

// parses a boolean value 
value* parseBool(arena* a, char** buf) {
	int val;

	// get boolean
	if(consume(buf, "#t")) {
		val = 1;
	} else if(consume(buf, "#f")) {
		val = 0;
	} else {
		logEvent(ERROR, LISP, "Invalid boolean value near %.20s", *buf);
		throw;
	}

	return makeBool(a, val); 
}

// parses a string value
value* parseString(arena* a, char** buf) {
	// get string
	const char* str = readString(buf);

	return makeString(a, str);
}

// parses a value
value* parseValue(arena* a, char** buf) {
	// eat stuff
	eatWhitespace(buf);
	eatComments(buf);

	// distinguish value type
	switch(**buf) {
		case '#':              return parseBool(a, buf);
		case '"':              return parseString(a, buf); 
		case '(':              return parseList(a, buf);
		case '\0':             return NULL;
		default:
			if(isDigit(*buf))  return parseNumber(a, buf);
			else               return parseSymbol(a, buf);
	}
}

// -- scripts

scriptContext* getScriptContext(char** buf) {
	INIT_JUMPS;

	// allocate script
	scriptContext* scr = xmalloc(sizeof(scriptContext));

	// initialize arenas
	scr->parseArena = newArena();
	scr->execArena = newArena();

	// parse script
	value* begin = NULL;
	try {
		// consider a script as a list
		value* head = NULL;
		value** tail = &head;

		// until end of list 
		for(;;) {
			// get value
			value* val = parseValue(&scr->parseArena, buf);
			if(!val) break; 

			// allocate cell
			value* cell = makeCons(&scr->parseArena, val, NULL);

			// append cell 
			*tail = cell;
			tail = &cell->cons.cdr;
		}

		// terminate list
		*tail = makeNil(); 

		// sugar into a begin block 
		begin = makeBegin(&scr->parseArena, head);

		// setup script
		scr->root = begin;
	} catch {
		logEvent(ERROR, LISP, "Couldn't parse script");
		freeArena(&scr->parseArena);
		freeArena(&scr->execArena);
		scr = NULL;
	}

	if(!scr) return NULL;

	// initialize environment
	initEnvironment(scr);

	// execute script from top level
	evaluateScript(scr);

	return scr;
}

void freeScriptContext(scriptContext* scr) {
	if(!scr) return;

	// free arenas
	freeArena(&scr->parseArena);
	freeArena(&scr->execArena);

	// free environment
	freeEnvironment(scr->env);

	free(scr);
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
void printNative(value* val __attribute__ ((unused))) {
	printf("(native)");
}

// prints a nil value
void printNil(value* val __attribute__ ((unused))) {
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

envFrame* newFrame(arena* a) {
	// allocate frame
	envFrame* frame = frAllocate(a, sizeof(envFrame));

	// setup frame
	frame->root = NULL;
	frame->arena = a;

	return frame;
}

void freeFrame(envFrame* frame) {
	if(!frame) return;

	// free only if on heap
	if(frame->arena) return;

	// go through all entries
	envEntry* head = frame->root;
	while(head) {
		// advance first
		envEntry* next = head->next;

		// free value
		freeValue(head->value);
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
	// if on heap, make a copy 
	if(!frame->arena) val = copyValue(val);

	// go through all entries first
    envEntry** tail = &frame->root;
	while(*tail) {
		envEntry* entry = *tail;

		// entry already exists
		if(strcmp(entry->key, key) == 0) {
			// if on heap, free old value 
			if(!frame->arena) freeValue(entry->value);
			
			// assign value
			entry->value = val;

			return;
		} 

		tail = &entry->next;
	
	} // entry doesn't exist 

	// create new entry 
	envEntry* nu = frAllocate(frame->arena, sizeof(envEntry)); 
	
	// setup entry
	nu->next = NULL;

	// copy key over
	int len = strlen(key);
	if(len >= SYM_SIZE) len = SYM_SIZE - 1;
	memcpy(nu->key, key, len);
	nu->key[len] = '\0';

	// assign value
	nu->value = val;

	// append entry
	*tail = nu; 
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
	envLink* link = frAllocate(frame->arena, sizeof(envLink)); 
    
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

	// get frame
	envFrame* frame = link->frame;
	
	// free link if on heap
	if(!frame->arena) free(link);

	return frame;
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

// get a nu environment
environment* newEnvironment() {
	// allocate environment
	environment* env = xmalloc(sizeof(environment));

	// setup environment
	env->root = NULL;
	env->scriptFrame = NULL;

	return env;
}

// forward declarations for initEnvironment
envFrame* initNativeEnvironment();
value* evaluateValue(arena* a, environment* env, value* val);

void initEnvironment(scriptContext* scr) {
	// allocate environment
	scr->env = newEnvironment();

	// push native frame
	envFrame* nativeFrame = initNativeEnvironment();
	pushFrame(scr->env, nativeFrame);

	// push script frame
	scr->env->scriptFrame = newFrame(NULL);
	pushFrame(scr->env, scr->env->scriptFrame);
}

void freeEnvironment(environment* env) {
	if(!env) return;

	// go through all frames 
	envLink* head = env->root;
	while(head) {
		// advance first
		envLink* next = head->next;

		// get frame
		envFrame* frame = head->frame;

		// free frame and link if on heap
		if(!frame->arena) {
			freeFrame(frame);
			free(head);
		}

		head = next;
	}

	free(env);
}

void printEnvironment(environment* env) {
	// go through all frames
	envLink* head = env->root;
	while(head) {
		printf("Frame arena: %p\n",
			head->frame->arena);
		printFrame(head->frame);
		if(head->next) printf("\n");

		head = head->next;
	}
}

// helper that cleans the environment for temporary frames
void cleanEnvironment(environment *env) {
    while(env->root && env->root->frame->arena) popFrame(env);
}

// -- natives

// typedef for arithmetic functions
typedef float (*arithmFunc)(float a, float b);

// definition of native arithmetic function
value* nativeArithm(arena* a, environment* env, value* args, arithmFunc func) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Arithmetic op. requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(lhs->type != VAL_NUMBER || args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to arithmetic op.");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(rhs->type != VAL_NUMBER || args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to arithmetic op.");
		throw;
	}

	// make result
	return makeNumber(a,
		func(lhs->number, rhs->number)
	);
}

// definition of native add function
float add(float a, float b) { return a + b; }
value* nativeAdd(arena* a, environment* env, value* args) {
	return nativeArithm(a, env, args, add);
}

// definition of native sub function
float sub(float a, float b) { return a - b; }
value* nativeSub(arena* a, environment* env, value* args) {
	return nativeArithm(a, env, args, sub);
}

// definition of native mul function
float mul(float a, float b) { return a * b; }
value* nativeMul(arena* a, environment* env, value* args) {
	return nativeArithm(a, env, args, mul);
}

// definition of native div function
float divs(float a, float b) { return a / b; }
value* nativeDiv(arena* a, environment* env, value* args) {
	return nativeArithm(a, env, args, divs);
}

// typedef for relational functions
typedef int (*relateFunc)(float a, float b);

// definition of native relational function
value* nativeRelate(arena* a, environment* env, value* args, relateFunc func) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Relational op. requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(lhs->type != VAL_NUMBER || args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to relational op.");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(rhs->type != VAL_NUMBER || args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to relational op.");
		throw;
	}

	// make result
	return makeBool(a,
		func(lhs->number, rhs->number)
	);
}

// definition of native less function
int less(float a, float b) { return a < b; }
value* nativeLess(arena* a, environment* env, value* args) {
	return nativeRelate(a, env, args, less);
}

// definition of native lessEqual function
int lessEqual(float a, float b) { return a <= b; }
value* nativeLessEqual(arena* a, environment* env, value* args) {
	return nativeRelate(a, env, args, lessEqual);
}

// definition of native greater function
int greater(float a, float b) { return a > b; }
value* nativeGreater(arena* a, environment* env, value* args) {
	return nativeRelate(a, env, args, greater);
}

// definition of native greaterEqual function
int greaterEqual(float a, float b) { return a >= b; }
value* nativeGreaterEqual(arena* a, environment* env, value* args) {
	return nativeRelate(a, env, args, greaterEqual);
}

// definition of native equal function
int equal(float a, float b) { return a == b; }
value* nativeEqual(arena* a, environment* env, value* args) {
	return nativeRelate(a,env, args, equal);
}

// definition of native notEqual function
int notEqual(float a, float b)  { return a != b; }
value* nativeNotEqual(arena* a, environment* env, value* args) {
	return nativeRelate(a, env, args, notEqual);
}

// definition of native define function
value* nativeDefine(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Define requires symbol to define");
		throw;
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
			logEvent(ERROR, EXEC, "Trying to define a desugared non-symbolic key");
			throw;
		}

		// make lambda
		value* lambdaSym = makeSymbol(a, "lambda");
		value* lambdaArgs = makeCons(a,
			params,
			body
		);
		value* lambda = makeCons(a,
			lambdaSym,
			lambdaArgs	
		);

		// rebuild args
		args = makeCons(a,
			lambda,
			makeNil()
		);
	}

	if(key->type != VAL_SYMBOL) {
		logEvent(ERROR, EXEC, "Trying to define a non-symbolic key");
		throw;
	}

	// get definition 
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Define requires symbol definition");
		throw;
	}
	if(args->cons.cdr->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Define takes exactly two arguments");
		throw;
	}
	value* val = evaluateValue(a, env, args->cons.car);

	// define
	addToFrame(env->scriptFrame, key->symbol, val);

	// return nil
	return makeNil();
}

// definition of native lambda function
value* nativeLambda(
	arena* a,
	environment* env __attribute__ ((unused)),
	value* args
) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Lambda requires a parameter list");
		throw;
	}

	// get parameter list
	value* params = args->cons.car;

	// validate parameters
	value* head = params;
	while(head->type != VAL_NIL) {
		if(head->type != VAL_CONS) {
			logEvent(ERROR, EXEC, "Malformed lambda parameter list");
			throw;
		}

		// make sure it's symbol
		value* param = head->cons.car;
		if(param->type != VAL_SYMBOL) {
			logEvent(ERROR, EXEC, "Lambda parameter is not a symbol");
			throw;
		}

		head = head->cons.cdr;
	}

	// body starts after parameter list
	value* body = args->cons.cdr;
	if(body->type == VAL_NIL) {
		logEvent(ERROR, EXEC, "Lambda requires a body");
		throw;
	}

	// create function
	function func;
	func.params = params;

	// sugar begin
	value* begin = makeBegin(a, body);
	func.body = begin;

	// return function
	return makeFunction(a, &func);
}

// definition of native begin function
value* nativeBegin(arena* a, environment* env, value* args) {
	// keep track of last result
	value* result = makeNil();

	// execute all found expressions
	while(args->type == VAL_CONS) {
		result = evaluateValue(a, env, args->cons.car);
		args = args->cons.cdr;
	}

	// make sure is end
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Malformed begin body");
		throw;
	}

	return result;
}

// definition of native quote function
value* nativeQuote(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* args
) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quote requires data");
		throw;
	}

	// get quoted data
	value* quote = args->cons.car;

	// exactly one argument
	if(args->cons.cdr->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Quote takes exactly one argument");
		throw;
	}

	return quote;
}

// definition of native list function
value* nativeList(arena* a, environment* env, value* args) {
	if(args->type == VAL_NIL) return args;

	// go through arguments
	value* result = NULL;
	value** tail = &result;
	while(args->type == VAL_CONS) {
		// evaluate argument
		value* val = evaluateValue(a, env, args->cons.car);

		// concatenate to list
		value* node = makeCons(a,
			val,
			makeNil()
		);

		// append node
		*tail = node;
		tail = &node->cons.cdr;

		// advance
		args = args->cons.cdr;
	}

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Malformed list arguments");
		throw;
	}

	return result;
}

// definition of native cons function
value* nativeCons(arena* a, environment* env, value* args) {
	// first value 
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Cons requires first argument");
		throw;
	}

	value* first = evaluateValue(a, env, args->cons.car); 
	args = args->cons.cdr;

	// second value 
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Cons requires second argument");
		throw;
	}

	value* second = evaluateValue(a, env, args->cons.car); 
	args = args->cons.cdr;

	// exactly two arguments
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Cons takes exactly two arguments");
		throw;
	}

	return makeCons(a,
		first,
		second
	); 
}

// definition of native car function
value* nativeCar(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Car requires an argument");
		throw;
	}

	// take pair
	value* pair = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Car takes exactly one argument");
		throw;
	}
	if(pair->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Car requires a pair");
		throw;
	}

	// car
	return pair->cons.car;
}

// definition of native cdr function
value* nativeCdr(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Cdr requires an argument");
		throw;
	}

	// take pair
	value* pair = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Cdr takes exactly one argument");
		throw;
	}
	if(pair->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Cdr requires a pair");
		throw;
	}

	// cdr
	return pair->cons.cdr;
}

// definition of native eval function
value* nativeEval(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Eval requires an argument");
		throw;
	}

	// take value 
	value* value = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Eval takes exactly one argument");
		throw;
	}

	// actually evaluate
	return evaluateValue(a, env, value);
}

// forward declaration for conditionals
int isTrue(value* val);

// definition of native if function
value* nativeIf(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "If requires condition");
		throw;
	}

	// get condition first
	value* condition = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	// first branch
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "If requires first branch");
		throw;
	}

	value* branch1 = args->cons.car;
	args = args->cons.cdr;

	// second branch
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "If requires second branch");
		throw;
	}

	value* branch2 = args->cons.car;
	args = args->cons.cdr;

	// exactly three arguments
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "If takes exactly three arguments");
		throw;
	}

	// execute appropiate branch
	if(isTrue(condition)) return evaluateValue(a, env, branch1);
	else                  return evaluateValue(a, env, branch2);
}

// definition of native and function
value* nativeAnd(arena* a, environment* env, value* args) {
	// first value
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "And requires first argument");
		throw;
	}

	value* first = args->cons.car; 
	args = args->cons.cdr;

	// second value
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "And requires second argument");
		throw;
	}

	value* second = args->cons.car;
	args = args->cons.cdr;

	// exactly two arguments
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "And takes exactly two arguments");
		throw;
	}

	// short circuit
	value* firstVal = evaluateValue(a, env, first);
	if(!isTrue(firstVal)) return firstVal;

	return evaluateValue(a, env, second);
}

// definition of native or function
value* nativeOr(arena* a, environment* env, value* args) {
	// first value
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Or requires first argument");
		throw;
	}

	value* first = args->cons.car; 
	args = args->cons.cdr;

	// second value
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Or requires second argument");
		throw;
	}

	value* second = args->cons.car;
	args = args->cons.cdr;

	// exactly two arguments
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Or takes exactly two arguments");
		throw;
	}

	// short circuit
	value* firstVal = evaluateValue(a, env, first);
	if(isTrue(firstVal)) return firstVal;

	return evaluateValue(a, env, second);
}

// definition of native not function
value* nativeNot(arena* a, environment* env, value* args) {
	// exactly one argument
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Not requires an argument");
		throw;
	}

	// take value 
	value* val = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;
	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Not takes exactly one argument");
		throw;
	}

	// take the opposite
	return makeBool(a, !isTrue(val));
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
	{ .key = "not",      .fn = nativeNot },

	// engine hooks
	{ .key = "log",      .fn = nativeLog   },
	{ .key = "error",    .fn = nativeError },
	{ .key = "warn",     .fn = nativeWarn  },
};

// number of native entries
#define NATIVE_ENTRIES (sizeof(nativeEntriesHelper) / sizeof(nativeEnvEntry))

// initializes the native environment
envFrame* initNativeEnvironment() {
	envEntry* head = NULL;
	envEntry* last = NULL;

	// go through all native entries
	for(size_t i = 0; i < NATIVE_ENTRIES; i++) {
		// get objects
		nativeEnvEntry* help  = &nativeEntriesHelper[i];
		value*          val   = makeNative(NULL, help->fn);
		envEntry*       entry = xmalloc(sizeof(envEntry));
		
		// initialize value and assign
		entry->value = val;
	
		// initialize entry
		memcpy(entry->key, help->key, SYM_SIZE);

		// concatenate
		if(last) last->next = entry;
		else head = entry;
		last = entry;
	}

	// terminate list
	last->next = NULL;

	// setup native frame 
	envFrame* nativeFrame = xmalloc(sizeof(envFrame));
	nativeFrame->arena = NULL;
	nativeFrame->root = head;

	return nativeFrame;
}

// -- evaluating

int isTrue(value* val) {
	int fls = val->type == VAL_BOOL && !val->boolean;
	return !fls;
}

value* evaluateSymbol(
	arena* a __attribute__ ((unused)),
	environment* env,
	value* val
) {
	envEntry* entry = queryEnvironment(env, val->symbol);
	if(entry) return entry->value;

	logEvent(ERROR, EXEC, "Symbol \"%s\" unknown", val->symbol);
	throw;
}

value* evaluateNumber(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
) {
	return val;
}

value* evaluateBool(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
) {
	return val;
}

value* evaluateString(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
) {
	return val;
}

value* applyFunction(arena* a, environment* env, function func, value* args) {
	// get parameters to bind
	value* params = func.params;

	// local environment frame
	envFrame* local = newFrame(a);

	// walk function params alongside cdr
	while(params && params->type != VAL_NIL
	&&    args   && args->type   != VAL_NIL) {
		// bind parameter
		value* arg = evaluateValue(a, env, args->cons.car);
		addToFrame(local, params->cons.car->symbol, arg);

		// advance
		params = params->cons.cdr;
		args   = args->cons.cdr;
	}

	// check sizes match
	if((params && params->type != VAL_NIL)
	|| (args   && args->type   != VAL_NIL)) {
		logEvent(ERROR, EXEC, "Wrong number of arguments");
		freeFrame(local);
		throw;
	}

	// push local frame
	pushFrame(env, local);

	// apply function
	value* result = NULL;
	result = evaluateValue(a, env, func.body);
	
	// pop local frame
	popFrame(env);
	
	return result;
}

value* evaluateList(arena* a, environment* env, value* val) {
	// get function symbol
	value* car = val->cons.car;

	// evaluate function symbol
	value* func = evaluateValue(a, env, car);
	
	// get arguments
	value* args = val->cons.cdr;

	// distinguish on callable types
	switch(func->type) {
		case VAL_FUNCTION: return applyFunction(a, env, func->func, args);
		case VAL_NATIVE:   return func->native(a, env, args);
		default: break;
	}

	logEvent(ERROR, EXEC, "Trying to call non-callable value");
	throw;
}

value* evaluateFunction(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
) {
	return val;
}

value* evaluateNative(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
) {
	return val;
}

value* evaluateNil(
	arena* a __attribute__ ((unused)),
	environment* env __attribute__ ((unused)),
	value* val
) {
	return val;
}

value* evaluateValue(arena* a, environment* env, value* val) {
	// distinguish value type
	switch(val->type) {
		case VAL_SYMBOL:   return evaluateSymbol(a, env, val);  
		case VAL_NUMBER:   return evaluateNumber(a, env, val);  
		case VAL_BOOL:     return evaluateBool(a, env, val);  
		case VAL_STRING:   return evaluateString(a, env, val);  
		case VAL_CONS:     return evaluateList(a, env, val);    
		case VAL_FUNCTION: return evaluateFunction(a, env, val);
		case VAL_NATIVE:   return evaluateNative(a, env, val);
		case VAL_NIL:      return evaluateNil(a, env, val);     
		default: return NULL;
	}
}

value* evaluateScript(scriptContext* scr) {
	INIT_JUMPS;

	// evaluate
	value* res = NULL;
	try {
		res = evaluateValue(&scr->execArena, scr->env, scr->root);
	} catch {
		logEvent(ERROR, EXEC, "Error evaluating script, things may break");
	}

	return res;
}

value* evaluateFuncFromScript(scriptContext* scr, const char* key) {
	INIT_JUMPS;

	// reset
	resetArena(&scr->execArena);
	cleanEnvironment(scr->env);
	
	// get key 
	envEntry* entry = queryEnvironment(scr->env, key);
	if(!entry) {
		logEvent(WARN, EXEC, "Script doesn't define key \"%s\"", key);
		return NULL;
	}

	// get function to evaluate
	value* val = entry->value;
	if(val->type != VAL_FUNCTION) {
		logEvent(ERROR, EXEC, "Script doesn't define key \"%s\" as function",
			key);
		return NULL;
	}

	// call function
	value* res = NULL;
	try {
		res = applyFunction(&scr->execArena, scr->env, val->func, makeNil());
	} catch {
		logEvent(ERROR, EXEC, "Couldn't apply function \"%s\"", key);
	}

	return res;
}
