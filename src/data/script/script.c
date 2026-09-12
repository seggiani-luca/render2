#include "script.h"
#include "../../parse/parse.h"
#include "../../exception/exception.h"
#include <stdlib.h>
#include <string.h>

void scriptPrint(void* dat) {
	script* scr = (script*)dat;
	printf("Script:");
	printValue(scr->scr->root);
}

void* script_import(FILE* file) {
	// load buffer		
	char* buf = slurpBufferPreprocess(file);
	if(!buf) {
		logEvent(ERROR, IO, "Couldn't load script");
		return NULL;
	}
	

	// initialize script 
	script* new_script = xmalloc(sizeof(script));
	memset(new_script, 0, sizeof(script));

	// parse script from buffer 
	char* ptr = buf;
	new_script->scr = parseScript(&ptr);
	if(!new_script->scr) {
		logEvent(ERROR, LISP, "Couldn't parse script");
		free(new_script);
		free(buf);
		return NULL;
	}

	// initialize the environment
	new_script->env = initEnvironment(new_script->scr->root);
	
	// cleanup
	free(buf);

	return new_script;
}

void script_free(void* dat) {
	if(!dat) return;

	script* scr = (script*)dat;
	freeScript(scr->scr);
	freeEnvironment(scr->env);

	free(dat);
}

// material handler implementations
DATA_TABLE_IMPL(script)
