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

// -- environment frames

// represents an environment frame
struct envFrame {
	// owner of frame
	void* owner;

	// are values owned?
	int valuesOwned; 

	// root of environment entries
	envEntry* root;
};
typedef struct envFrame envFrame;

// gets a new environment frame
envFrame* newFrame(void* owner, int valuesOwned);

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

	// arena allocator for execution
	arena arena;
};

// pushes a top frame to an environment
void pushFrame(environment* env, envFrame* frame);

// pops the top frame from an environment
envFrame* popFrame(environment* env);

// gets an entry from an environment
envEntry* queryEnvironment(environment* env, const char* key);

// forward declaration for initEnvironment
typedef struct script script;

// initializes an environment from a script
environment* initEnvironment(script* scr); 

// frees a script's environment 
void freeEnvironment(environment* env); 

// prints an environment
void printEnvironment(environment* env);

// -- scripts

// represents a Scheme script
struct script {
	// root of CONS making up script
	value* root;
};

// parses a Scheme script 
script* parseScript(char** buf);

// frees a Scheme script
void freeScript(script* scr);

// prints a Scheme script
void printScript(script* scr);

// -- evaluating

// evaluates a script value
value* evaluateValue(environment* env, value* val);

#endif
