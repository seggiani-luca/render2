#include "parse.h"
#include "../exception/exception.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// -- utils

// size of file line
#define FILE_LINE_SIZ 1024

// steps for file buffer increase
#define BUF_SIZE_STEP 1024

char* slurpBuffer(FILE* f) {
	// read file size
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	// allocate buffer
	char *buf = xmalloc(size + 1);

	// store file in buffer and terminate
	fread(buf, 1, size, f);
	buf[size] = '\0';

	return buf;
}

char* slurpBufferPreprocess(FILE* f) {
	char line[FILE_LINE_SIZ];
	
	size_t capacity = BUF_SIZE_STEP;
	size_t used = 0;
	
	// buffer for file
	char* buf = xmalloc(sizeof(char) * capacity);

	while(fgets(line, FILE_LINE_SIZ, f)) {
		char* ptr = line;

		// check if preprocessor directive
		int shouldFree = 0;
		if(consume(&ptr, "#include")) {
			// get path
			eatWhitespace(&ptr);
			const char* path = readString(&ptr);

			// open file
			FILE* includeFile = fopen(path, "r");
			if(!includeFile) {
				logEvent(ERROR, IO, "Couldn't load included file %s", path);
				free(buf);
				return NULL;
			}

			// recursive includes
			char* includeBuf = slurpBufferPreprocess(includeFile);
			fclose(includeFile);
			if(!includeBuf) {
				free(buf);
				return NULL;
			}
			shouldFree = 1;
			ptr = includeBuf;
		}

		// line length
		size_t len = strlen(ptr);

		// realloc if needed 
		while(used + len + 1 > capacity) {
			capacity += BUF_SIZE_STEP;
			buf = xrealloc(buf, sizeof(char) * capacity);
		}

		// copy over
		memcpy(buf + used, ptr, len);
		used += len;
	
		// free if needed
		if(shouldFree) free(ptr);
	}

	// terminate
	buf[used] = '\0';

	return buf;
}

// -- parsing

int isWhitespace(char c) {
	return c == '\n'
	    || c == '\r'
	    || c == '\t'
	    || c == ' ';
}

int isDigit(char* buf) {
	// first check this digit
	char c0 = *buf;
	if(c0 >= '0' && c0 <= '9') return 1;

	// then check if positive/negative
	char c1 = *(buf + 1);
	if(c0 == '+' || c0 == '-')
		if(c1 >= '0' && c1 <= '9') return 1;

	return 0;
}

void eatWhitespace(char** buf) {
	while(buf
	   && isWhitespace(**buf)) {
		(*buf)++;
	}
}

void expect(char** buf, const char* key) {
	int len = strlen(key);
	if(strncmp(*buf, key, len)) {
		logEvent(ERROR, JSON, "Unexpected token near %.20s", *buf);
		throw;	
	} 
	
	*buf += len;
}

int consume(char** buf, const char* key) {
	int len = strlen(key);
	if(strncmp(*buf, key, len)) return 0;
	
	*buf += len;
	return 1;
}

const char* readString(char** buf) {
	// begin string
	expect(buf, "\"");

	// find end of string
	char* next = strchr(*buf, '\"');
	if(next == NULL) {
		logEvent(ERROR, JSON, "Non terminated string near %.20s", *buf);
		throw;	
	} 

	// get return
	const char* ret = *buf;
	*next = '\0';

	// advance
	*buf = next + 1;

	return ret;
}
