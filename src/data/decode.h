#ifndef DATA_DECODE_H
#define DATA_DECODE_H

#include <stdio.h>
#include <string.h>

// checks an .obj/.mtl line for a given key. eats the key
static inline char* checkKey(char* line, const char* key) {
	// only if key matches
	int keySiz = strlen(key);
	if (strncmp(line, key, keySiz) == 0) {
		return line += keySiz;
	}

	return 0;
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
			sscanf(line, "%f %f",
				&buf[*cur], 
				&buf[*cur + 1]
			);
			break;

		case 3:
			sscanf(line, "%f %f %f",
				&buf[*cur], 
				&buf[*cur + 1],
				&buf[*cur + 2]
			);
			break;

		default:
			break;  // good enough for the demo lol
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
			sscanf(line, "%f",
				buf 
			);
			break;

		case 2:
			sscanf(line, "%f %f",
				buf,
				buf + 1
			);
			break;

		case 3:
			sscanf(line, "%f %f %f",
				buf,
				buf + 1,
				buf + 2
			);
			break;

		default:
			break;  // good enough for the demo lol
	}

	return 1;
}

// parses an .obj/.mtl line for a given key, updating a string if needed
static inline int parseStringKey(
	char* line,
	const char* key,
	char* str
) {
	// only if key matches
	line = checkKey(line, key);
	if(!line) return 0;
	
	// parse line
	sscanf(line, "%255s", str);

	return 1;
}

#endif
