#ifndef JSON_H
#define JSON_H

#include <stdio.h>

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
jsonElement* newJsonString(const char* val);

// creates a new JSON number element
jsonElement* newJsonNumber(float val);

// creates a new JSON object element
jsonElement* newJsonObject();

// creates a new JSON array element
jsonElement* newJsonArray();

// creates a new JSON boolean element
jsonElement* newJsonBoolean(int val);

// creates a new JSON null element
jsonElement* newJsonNull(); 

// -- serialization / deserialization

// print a JSON object to a file: the OS handles dynamic size writes
void serializeJsonObject(FILE* file, jsonElement* elem);

// parses a JSON object from a buffer: slurp before deserializing
jsonElement* deserializeJsonObject(char** buf);

// frees a JSON object
void freeJsonObject(jsonElement* elem);

#endif
