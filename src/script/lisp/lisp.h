#ifndef LISP_H
#define LISP_H

#include "../../arena/arena.h"

// -- values

// size of symbols
#define SYM_SIZE 32

// forward declarations for functions
typedef struct environment environment;
typedef struct value value;

// native function type
typedef value* (*nativeFn)(arena* a, environment* env, value* args); 

// represents a function
typedef struct function {
	// CONS of parameters
	value* params;

	// body of the function 
	value* body;
} function;

// enum of value types
typedef enum {
	VAL_SYMBOL,
	VAL_NUMBER,
	VAL_BOOL,
	VAL_STRING,
	VAL_CONS,
	VAL_FUNCTION,
	VAL_NATIVE,
	VAL_NIL	
} valueType;

// represents any Scheme value
struct value {
	// type of value
	valueType type;

	// payload of value 
	union {
		// symbol value
	    char symbol[SYM_SIZE];

		// floating-point number value
	    float number;
		
		// integer (boolean) value 
		int boolean;

		// string value
		const char* string;

		// CAR/CDR CONS list element
	    struct {
	        struct value *car; // first element of list
	        struct value *cdr; // continuation of list
	    } cons; // list CONStruct

		// user function
		function func;

		// native function
		nativeFn native;
	};
};

// -- environment entries 

// represents an environment entry
struct envEntry {
	// symbol this entry defines
	char key[SYM_SIZE];

	// payload of entry
	value* value;

	// next entry in environment
	struct envEntry* next;
};
typedef struct envEntry envEntry;

// -- environment frames

// represents an environment frame
struct envFrame {
	// arena to use, NULL signals heap
	arena* arena;

	// root of environment entries
	envEntry* root;
};
typedef struct envFrame envFrame;

// gets a new environment frame
envFrame* newFrame(arena* a);

// frees an environment frame
void freeFrame(envFrame* frame);

// gets an entry from an environment frame
envEntry* queryFrame(envFrame* frame, const char* key);

// adds an a key-value to an environment frame
void addToFrame(envFrame* frame, const char* key, value* val);
	
// -- environments

// represents an environment frame link to an environment
struct envLink {
	// the frame to link
	envFrame* frame;

	// pointer of next frame
    struct envLink* next;
};
typedef struct envLink envLink;

// represents an environment
struct environment {
	// root of environment frames
	envLink* root;

	// own frame of this environment's script 
	envFrame* scriptFrame;
};

// pushes a top frame to an environment
void pushFrame(environment* env, envFrame* frame);

// pops the top frame from an environment
envFrame* popFrame(environment* env);

// gets an entry from an environment
envEntry* queryEnvironment(environment* env, const char* key);

// forward declaration for initEnvironment
typedef struct scriptContext scriptContext;

// initializes an environment from a script
void initEnvironment(scriptContext* scr); 

// frees a script's environment 
void freeEnvironment(environment* env); 

// -- scripts

// store a script's context, that is it's storage arenas and environment 
struct scriptContext{
	// parse arena
	arena parseArena;

	// execution arena 
	arena execArena;

	// environment of script
	environment* env;

	// root of script
	value* root;
};

// parses a Scheme script and returns its context
scriptContext* getScriptContext(char** buf);

// frees a Scheme script
void freeScriptContext(scriptContext* scr);

// -- debug printing

// length of CONS list to break off when pretty-printing
#define PP_CONS_LEN 1

// prints a script value 
void printValue(value* val);

// pretty-prints a script value 
void prettyPrintValue(value* val);

// prints an environment
void printEnvironment(environment* env);

// -- evaluating

// evaluates a full script 
value* evaluateScript(scriptContext* scr);

// evaluates a function from a script's environment 
value* evaluateFuncFromScript(scriptContext* scr, const char* key); 

#endif
