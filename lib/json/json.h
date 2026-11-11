/**
 * Minimal JSON serialization / deserialization library.
 * Example usage:
 *
 * // TODO
 *
 * 2026 - Luca Seggiani
 */

#ifndef JSON_H
#define JSON_H

#include <stdio.h>

// -- setup

/*
 * Allocator template to feed to deserialization functions.
 *
 * This is meant to allow for the caller to do cleanup on the objects allocated
 * by the library in case of library failure.
 *
 * It can also be used to allocate JSON objects in arbitrary memory allocators.
 */
typedef void* (*jsonAllocator)(size_t);

/*
 * Deallocator for the above allocator. 
 */
typedef void (*jsonDeallocator)(void*);

// -- definitions

// type of JSON element
typedef enum {
	JSON_STRING,
	JSON_NUMBER,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BOOLEAN,
	JSON_NULL
} jsonType;

// represents JSON element
typedef struct jsonElement {
	// key of value 
	const char* key;

	// type of element
	jsonType type;

	// payload of element
	union {
		// string value
		const char* string;

		// floating-point number value 
		float number;
		
		// integer (boolean) value 
		int boolean;

		// head of object children or array list
		struct jsonElement* head;
	} value;

	// next pointer
	struct jsonElement* next;
} jsonElement; 

// -- getting 

// assumes a JSON element to be an object, and returns the value at a key
jsonElement* getJsonAtKey(const jsonElement* elem, const char* key);

// assumes a JSON element to be an array, and returns its first element 
jsonElement* getJsonHead(const jsonElement* elem);

// assumes a JSON element to be an array member, and returns its next element
jsonElement* getJsonNext(const jsonElement* elem);

// assumes a JSON element to be a string and returns its value
const char* getJsonString(const jsonElement* elem);

// assumes a JSON element to be a float and returns its value
float getJsonNumber(const jsonElement* elem);

// assumes a JSON element to be a boolean and returns its value
int getJsonBoolean(const jsonElement* elem);

// is a JSON element null?
int isJsonNull(const jsonElement* elem);

// -- setting

// adds a JSON element to an object at a key
jsonElement* addJsonAtKey(jsonElement* elem, jsonElement* n, const char* key);

// adds a JSON element to the tail of to an array
jsonElement* addJsonAtTail(jsonElement* elem, jsonElement* n);

// creates a new JSON string element
jsonElement* newJsonString(jsonAllocator a, const char* val);

// creates a new JSON number element
jsonElement* newJsonNumber(jsonAllocator a, float val);

// creates a new JSON object element
jsonElement* newJsonObject(jsonAllocator a);

// creates a new JSON array element
jsonElement* newJsonArray(jsonAllocator a);

// creates a new JSON boolean element
jsonElement* newJsonBoolean(jsonAllocator a, int val);

// creates a new JSON null element
jsonElement* newJsonNull(jsonAllocator a); 

// -- serialization / deserialization

// print a JSON object to a file: let the OS handle dynamic size writes
void serializeJsonObject(FILE* file, jsonElement* elem);

// parses a JSON object from a buffer: slurp before deserializing
jsonElement* deserializeJsonObject(jsonAllocator a, char** buf);

// frees a JSON object
void freeJsonObject(jsonDeallocator d, jsonElement* elem);

// -- errors

/**
 * Reports an error that happened in the library, for instance from a returned
 * NULL.
 *
 * @return a string representing the last error that happened
 */
const char* jsonError();

#endif
