#include "script.h"
#include "../parse/parse.h"
#include <stdlib.h>
#include <stdio.h>

#define PATH "test.scm"

int main() {
	// get file buffer
	char* buf = slurpBuffer(PATH);
	if(!buf) return 1;

	// parse script from file
	char* ptr = buf;
	script scr = parseScript(&ptr);

	// first print the script
	printf("== PARSING SCRIPT ==\n");
	printScript(scr);
	printf("\n");

	// then initialize environment
	printf("== INITIALIZING ENVIRONMENT ==\n");
	environment* env = initEnvironment(scr);
	printf("All good.\n");
	printf("\n");

	// inspect the environment
	printf("== INSPECTING ENVIRONMENT ==\n");
	printEnvironment(env);
	printf("\n");

	// cleanup
	freeEnvironment(env);
	freeScript(scr);
	free(buf);
}
