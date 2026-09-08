#ifndef SCRIPT_H
#define SCRIPT_H

#include "arena/arena.h"

// -- values

// size of symbols
#define SYM_SIZE 32

// forward declarations for functions
typedef struct environment environment;
typedef struct value value;

// native function type
typedef value* (*nativeFn)(environment* env, value* args); 

// represents a function
typedef struct function {
	// CONS of parameters
	value* params;

	// body of the function 
	value* body;
} function;

// enum of value types
typedef enum {
	VAL_NUMBER,
	VAL_SYMBOL,
	VAL_CONS,
	VAL_FUNCTION,
	VAL_NATIVE,
	VAL_NIL	
} valueType;

// represents any Scheme value
struct value {
	// owner environment, if it exists
	environment* owner;

	// type of value
	valueType type;

	// payload of value 
	union {
		// symbol value
	    char symbol[SYM_SIZE];

		// floating-point number value
	    float number;

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

// -- environments

// size of environment arena allocator
#define ARENA_SIZ (sizeof(value) * 512)

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

// represents an environment 
struct environment {
	// root of environment entries
	envEntry* root;
	
	// arena allocator for execution
	arena arena;	
};

// gets an entry from an environment
envEntry* queryEnv(environment* env, const char* key);

// adds an entry to an environment
void addEnv(environment* env, const char* key, value* val);

// -- scripts

// represents a Scheme expression within a script
struct expression {
	// actual expression value
	value* val;

	// next expression in script 
	struct expression* next;
};
typedef struct expression expression; 

// represents a Scheme script
typedef expression* script;

// parses a Scheme script 
script parseScript(char** buf);

// frees a Scheme script
void freeScript(script scr);

// -- printing

// prints a Scheme script
void printScript(script scr);

// prints an environment
void printEnvironment(environment* env);

// -- evaluating

// initializes an environment from a script
environment* initEnvironment(script scr); 

// frees a script's environment 
void freeEnvironment(environment* env); 

// evaluates a script value
value* evaluateValue(environment* env, value* val);

#endif
