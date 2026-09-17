#ifndef DATA_DECODE_H
#define DATA_DECODE_H

#include <stdio.h>
#include <string.h>

// checks an .obj/.mtl line for a given key. eats the key
static inline char* checkKey(char* line, const char* key) {
	// only if key matches
	size_t keySiz = strlen(key);

	// check for the key
	if(strncmp(line, key, keySiz) != 0) return NULL;

	// check for whitespace
	if(line[keySiz] != '\0'
	&& line[keySiz] != ' '
	&& line[keySiz] != '\t') {
		return NULL;
	}

	return line + keySiz;
}

// parses an .obj/.mtl line for a given key, updating a float buffer if needed 
static inline int parseFloatBufKey(
	char* line,
	const char* key,
	int dim,
	int* cur,
	float* buf
) {
	// only if key matches
	line = checkKey(line, key);
	if(!line) return 0;
	
	// parse line
	switch(dim) {
		case 2:
			if(sscanf(line, "%f %f",
				&buf[*cur], 
				&buf[*cur + 1]) != 2) return 0;
			break;

		case 3:
			if(sscanf(line, "%f %f %f",
				&buf[*cur], 
				&buf[*cur + 1],
				&buf[*cur + 2]) != 3) return 0;
			break;

		default: return 0; // good enough for the demo lol
	}

	// advance buffer
	*cur += dim;

	return 1;
}

// parses an .obj/.mtl line for a given key, updating a float if needed
static inline int parseFloatKey(
	char* line,
	const char* key,
	int dim,
	float* buf
) {
	// only if key matches
	line = checkKey(line, key);
	if(!line) return 0;
	
	// parse line
	switch(dim) {
		case 1:
			if(sscanf(line, "%f", buf) != 1) return 0;
			break;

		case 2:
			if(sscanf(line, "%f %f", buf, buf + 1)) return 0;
			break;

		case 3:
			if(sscanf(line, "%f %f %f", buf, buf + 1, buf + 2)) return 0;
			break;

		default: return 0; // good enough for the demo lol
	}

	return 1;
}

// parses an .obj/.mtl line for a given key, updating a string if needed
static inline int parseStringKey(
	char* line,
	const char* key,
	size_t strSiz,
	char* str
) {
	// only if key matches
	line = checkKey(line, key);
	if (!line) return 0;

	// skip whitespace
	while (*line == ' ' || *line == '\t') line++;

	// get len of string
	size_t len = strcspn(line, " \t\r\n");
	if (len >= strSiz) len = strSiz - 1; 

	// copy over
	memcpy(str, line, len);
	str[len] = '\0';

	return 1;
}

#endif
