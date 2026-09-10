#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

// -- utils

// reads a file into a buffer
char* slurpBuffer(FILE* f);

// -- parsing

// checks if character is whitespace
int isWhitespace(char c);

// checks if character is digit
int isDigit(char* buf);

// eats up the whitespace in the given buffer
void eatWhitespace(char** buf);

// strictly expects a certain keyword in the given buffer and consumes it
void expect(char** buf, const char* key);

// expects a certain keyword in the given buffer and consumes it
int consume(char** buf, const char* key);

// expects a string in the given buffer, consumes it and returns it 
const char* readString(char** buf);

#endif
