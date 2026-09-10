#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// -- utils

char* slurpBuffer(FILE* f) {
	// read file size
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	// allocate buffer
	char *buf = malloc(size + 1);

	// store file in buffer and terminate
	fread(buf, 1, size, f);
	buf[size] = '\0';

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
		printf("Unexpected token near %.20s\n", *buf);
		exit(1);
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
		printf("Non terminated string near %.20s\n", *buf);
		exit(1);
	} 

	// get return
	const char* ret = *buf;
	*next = '\0';

	// advance
	*buf = next + 1;

	return ret;
}
