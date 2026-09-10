#include "script.h"
#include "../../parse/parse.h"
#include <stdlib.h>
#include <string.h>

void scriptPrint(void* dat) {
	script* scr = (script*)dat;
	printf("Script:");
	printValue(scr->root);
}

void* script_import(FILE* file) {
	// initialize material 
	script* new_script = malloc(sizeof(script));
	memset(new_script, 0, sizeof(script));

	// load buffer		
	new_script->buf = slurpBuffer(file);
	if(!new_script->buf) return NULL;
	
	// parse script from buffer 
	char* ptr = new_script->buf;
	new_script->root = parseScript(&ptr);

	// initialize the environment
	new_script->env = initEnvironment(new_script->root);
	
	return new_script;
}

void script_free(void* dat) {
	if(!dat) return;

	script* scr = (script*)dat;
	freeValue(scr->root);
	freeEnvironment(scr->env);
	free(scr->buf);

	free(dat);
}

// material handler implementations
DATA_TABLE_IMPL(script)
