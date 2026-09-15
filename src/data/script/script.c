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
	// initialize script 
	script* new_script = xmalloc(sizeof(script));
	memset(new_script, 0, sizeof(script));
	
	// load buffer		
	new_script->buf = slurpBufferPreprocess(file);
	if(!new_script->buf) {
		logEvent(ERROR, IO, "Couldn't load script");
		return NULL;
	}
	
	// parse script from buffer 
	char* ptr = new_script->buf;
	new_script->scr = parseScript(&ptr);
	if(!new_script->scr) {
		logEvent(ERROR, LISP, "Couldn't parse script");
		free(new_script->buf);
		free(new_script);
		return NULL;
	}

	// initialize the environment
	new_script->env = initEnvironment(new_script->scr->root);

	return new_script;
}

void script_free(void* dat) {
	if(!dat) return;

	script* scr = (script*)dat;
	freeScript(scr->scr);
	freeEnvironment(scr->env);
	free(scr->buf)
;
	free(dat);
}

// material handler implementations
DATA_TABLE_IMPL(script)
