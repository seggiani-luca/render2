#ifndef DATA_SCRIPT_H
#define DATA_SCRIPT_H

#include "../data.h"
#include "../../script/script.h"

// -- script 

// material data type
typedef struct {
	// text buffer
	char* buf;

	// AST of script 
	scriptVal* scr;

	// script environment
	environment* env;
} script;

// print script info
void scriptPrint(void* dat);

// script handler declarations
DATA_TABLE_DECL(script)

#endif
