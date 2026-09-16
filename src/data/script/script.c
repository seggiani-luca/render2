#include "script.h"
#include "../../parse/parse.h"
#include "../../exception/exception.h"
#include <stdlib.h>
#include <string.h>

void scriptPrint(void* dat) {
	script* scr = (script*)dat;
	printf("Script:");
	printValue(scr->ctx->root);
}

void* script_import(FILE* file) {
	// initialize script 
	script* new_script = xmalloc(sizeof(script));
	memset(new_script, 0, sizeof(script));
	
	// load buffer		
	new_script->buf = slurpBufferPreprocess(file);
	if(!new_script->buf) {
		logEvent(ERROR, IO, "Couldn't load script");
		free(new_script->buf);
		return NULL;
	}
	
	// parse script from buffer 
	char* ptr = new_script->buf;
	new_script->ctx = getScriptContext(&ptr);
	if(!new_script->ctx) {
		logEvent(ERROR, LISP, "Couldn't load script");
		free(new_script->buf);
		free(new_script);
		return NULL;
	}

	return new_script;
}

void script_free(void* dat) {
	if(!dat) return;

	script* scr = (script*)dat;
	freeScriptContext(scr->ctx);
	freeEnvironment(scr->env);
	free(scr->buf);

	free(dat);
}

// material handler implementations
DATA_TABLE_IMPL(script)
