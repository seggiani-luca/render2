#ifndef DATA_SCRIPT_H
#define DATA_SCRIPT_H

#include "../data.h"
#include "../../script/script.h"

// -- script 

// material data type
typedef struct {
	// buffer of code
	char* buf;

	// AST of script 
	value* root;

	// script environment
	environment* env;
} script;

// print script info
void scriptPrint(void* dat);

// script handler declarations
DATA_TABLE_DECL(script)

#endif
