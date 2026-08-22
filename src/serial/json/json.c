#include "json.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// -- getting 

jsonElement* getJsonAtKey(const jsonElement* elem, const char* key) {
	// go through all children
	jsonElement* cur = elem->value.head;
	while(cur) {
		if(strcmp(cur->key, key) == 0) return cur;
		cur = cur->next;
	}

	return NULL;
}

jsonElement* getJsonHead(const jsonElement* elem) {
	return elem->value.head;
}

jsonElement* getJsonNext(const jsonElement* elem) {
	return elem->next;
}

const char* getJsonString(const jsonElement* elem) {
	return elem->value.string;
}

float getJsonNumber(const jsonElement* elem) {
	return elem->value.number;
}

int getJsonBoolean(const jsonElement* elem) {
	return elem->value.boolean;
}

int isJsonNull(const jsonElement* elem) {
	return elem->type == JSON_NULL;
}

// -- setting

jsonElement* addJsonAtKey(jsonElement* elem, jsonElement* n, const char* key) {	
	// add as last element 
	jsonElement **cur = &elem->value.head;
	while(*cur) cur = &(*cur)->next;
	*cur = n;

	// set key
	n->key = key;

	return elem;
}

jsonElement* addJsonAtTail(jsonElement* elem, jsonElement* n) {
	// add as last element 
	jsonElement **cur = &elem->value.head;
	while(*cur) cur = &(*cur)->next;
	*cur = n;

	return elem;
}

jsonElement* newJsonString(const char* val) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_STRING;
	new->next = NULL;
	new->value.string = val;

	return new;
}

jsonElement* newJsonNumber(float val) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_NUMBER;
	new->next = NULL;
	new->value.number = val;

	return new;
}

jsonElement* newJsonObject() {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_OBJECT;
	new->next = NULL;
	new->value.head = NULL;

	return new;
}

jsonElement* newJsonArray() {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_ARRAY;
	new->next = NULL;
	new->value.head = NULL;

	return new;
}

jsonElement* newJsonBoolean(int val) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_BOOLEAN;
	new->next = NULL;
	new->value.boolean = val;

	return new;
}

jsonElement* newJsonNull() {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_NULL;
	new->next = NULL;

	return new;
}

// -- serialization

// inserts tabs to reach given depth
void tabulate(FILE* file, int depth) {
	for(int i = 0; i < depth; i++) fprintf(file, "\t");
}

// prints a string to a file 
void printString(FILE* file, const char* str) {
	fprintf(file, "\"%s\"", str);
}

// forward declarations for JSON object printing 
void serializeJsonKey(FILE* file, jsonElement* elem);
void serializeJsonValue(FILE* file, jsonElement* elem, int depth);

// actually serializes a JSON object to a file, taking depth into account
void doSerializeJsonObject(FILE* file, jsonElement* elem, int depth) {
	if(!elem->value.head) {
		fprintf(file, "{}");
		return;
	}

	// open object
	fprintf(file, "{\n");
	
	// go through all elements in object
	jsonElement* cur = elem->value.head;
	while(cur) {
		tabulate(file, depth + 1);

		// serialize key and value
		serializeJsonKey(file, cur);
		fprintf(file, " : ");
		serializeJsonValue(file, cur, depth + 1);

		// comma terminate
		if(cur->next) fprintf(file, ",");
		fprintf(file, "\n");

		cur = cur->next;
	}

	// close object
	tabulate(file, depth);
	fprintf(file, "}");
}

void serializeJsonObject(FILE* file, jsonElement* elem) {
	doSerializeJsonObject(file, elem, 0);
}

// serializes a JSON array to a file
void serializeJsonArray(FILE* file, jsonElement* elem, int depth) {
	if(!elem->value.head) {
		fprintf(file, "[]");
		return;
	}

	// open array 
	fprintf(file, "[\n");

	// go through all elements in array 
	jsonElement* cur = elem->value.head;
	while(cur) {
		tabulate(file, depth + 1);
		
		// serialize value
		serializeJsonValue(file, cur, depth + 1);

		// comma terminate
		if(cur->next) fprintf(file, ",");
		fprintf(file, "\n");

		cur = cur->next;
	}

	// close array
	tabulate(file, depth);
	fprintf(file, "]");	
}

// serializes a JSON string to a file
void serializeJsonString(FILE* file, jsonElement* elem) {
	printString(file, elem->value.string);
}

// serializes a JSON boolean to a file
void serializeJsonBool(FILE* file, jsonElement* elem) {
	fprintf(file, "%s", elem->value.boolean ? "true" : "false");
}

// serialiezs a JSON null to a file
void serializeJsonNull(FILE* file) {
	fprintf(file, "null");
}

// serializes a JSON number to a file
void serializeJsonNumber(FILE* file, jsonElement* elem) {
	fprintf(file, "%f", elem->value.number);
}

// serializes a JSON key to a file
void serializeJsonKey(FILE* file, jsonElement* elem) {
	printString(file, elem->key);
}

// serializes a JSON value to a file
void serializeJsonValue(FILE* file, jsonElement* elem, int depth) {
	// distinguish value type
	switch(elem->type) {
		case JSON_OBJECT:  doSerializeJsonObject(file, elem, depth); break;
		case JSON_ARRAY:   serializeJsonArray(file, elem, depth);    break;
		case JSON_STRING:  serializeJsonString(file, elem);          break;
		case JSON_BOOLEAN: serializeJsonBool(file, elem);            break;
		case JSON_NULL:    serializeJsonNull(file);                  break;
		case JSON_NUMBER:  serializeJsonNumber(file, elem);          break;
		default: break;
	}
}

// -- deserialization 

// eats up the whitespace in the given buffer
void eatWhitespace(char** buf) {
	while(**buf == '\n'
	|| **buf == '\r'
	|| **buf == '\t'
	|| **buf == ' ') {
		(*buf)++;
	}
}

// strictly expects a certain keyword in the given buffer and consumes it
void expect(char** buf, const char* key) {
	int len = strlen(key);
	if(strncmp(*buf, key, len)) {
		printf("JSON parsing error near %.20s\n", *buf);
		exit(1);
	} 
	
	*buf += len;
}

// expects a certain keyword in the given buffer and consumes it
int consume(char** buf, const char* key) {
	int len = strlen(key);
	if(strncmp(*buf, key, len)) return 0;
	
	*buf += len;
	return 1;
}

// expects a string in the given buffer, consumes it and returns it 
const char* readString(char** buf) {
	// begin string
	expect(buf, "\"");

	// find end of string
	char* next = strchr(*buf, '\"');
	if(next == NULL) {
		printf("Non terminated string in JSON near %.20s\n", *buf);
		exit(1);
	} 

	// get return
	const char* ret = *buf;
	*next = '\0';

	// advance
	*buf = next + 1;

	return ret;
}

// forward declarations for JSON object parsing
const char* deserializeJsonKey(char** buf);
jsonElement* deserializeJsonValue(char** buf);

jsonElement* deserializeJsonObject(char** buf) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_OBJECT;
	new->value.head = NULL;
	new->next = NULL;

	// also free key
	new->key = NULL;

	// begin object 
	expect(buf, "{");
	eatWhitespace(buf);

	// check for empty objects
	if(consume(buf, "}")) return new;

	// until end of object
	for(;;) {
		// get element key
		const char* key = deserializeJsonKey(buf);

		// key-value separator
		eatWhitespace(buf);
		expect(buf, ":");
		eatWhitespace(buf);

		// get element value
		jsonElement* elem = deserializeJsonValue(buf);
		elem->key = key;

		// append element to object
		jsonElement **cur = &new->value.head;
		while(*cur) cur = &(*cur)->next;
		*cur = elem;

		// check for last element 
		eatWhitespace(buf);
		if(consume(buf, "}")) break;

		// element separator
		expect(buf, ",");
		eatWhitespace(buf);
	}

	return new;
}

// parses a JSON array from a buffer
jsonElement* deserializeJsonArray(char** buf) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_ARRAY;
	new->value.head = NULL;
	new->next = NULL;
	
	// begin array 
	expect(buf, "[");
	eatWhitespace(buf);
	
	// check for empty arrays 
	if(consume(buf, "]")) return new;

	// until end of array 
	for(;;) {
		// get element value
		jsonElement* elem = deserializeJsonValue(buf);
		elem->key = NULL;

		// append element to array 
		jsonElement **cur = &new->value.head;
		while(*cur) cur = &(*cur)->next;
		*cur = elem;

		// check for last element 
		eatWhitespace(buf);
		if(consume(buf, "]")) break;

		// element separator
		expect(buf, ",");
		eatWhitespace(buf);
	}

	return new;
}

// parses a JSON string from a buffer
jsonElement* deserializeJsonString(char** buf) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_STRING;
	new->next = NULL;

	// get string
	const char* str = readString(buf);
	new->value.string = str;

	return new;
}

// parses a JSON boolean from a buffer
jsonElement* deserializeJsonBool(char** buf) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_BOOLEAN;
	new->next = NULL;

	// get boolean
	if(consume(buf, "true")) {
		new->value.boolean = 1;
	} else if(consume(buf, "false")) {
		new->value.boolean = 0;
	} else {
		printf("Invalid JSON boolean value near %.20s\n", *buf);
		exit(1);
	}
	
	return new;
}

// parses a JSON null from a buffer
jsonElement* deserializeJsonNull(char** buf) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_NULL;
	new->next = NULL;

	// get NULL
	expect(buf, "null");

	return new;
}

// parses a JSON numbe from a buffer
jsonElement* deserializeJsonNumber(char** buf) {
	// allocate element
	jsonElement* new = malloc(sizeof(jsonElement));
	if(!new) return NULL;
	
	// setup element 
	new->type = JSON_NUMBER;
	new->next = NULL;

	// get number
	char *end;
	new->value.number = strtof(*buf, &end);

	// validate trash reads
	if (end == *buf) {
		printf("Invalid number in JSON near %.20s\n", *buf);
		exit(1);
	}

	// advance 
	*buf = end;

	return new;
}

// parses a JSON key from a buffer
const char* deserializeJsonKey(char** buf) {
	return readString(buf);	
}

// parses a JSON value from a buffer
jsonElement* deserializeJsonValue(char** buf) {
	eatWhitespace(buf);

	// distinguish value type
	switch(**buf) {
		case '{': return deserializeJsonObject(buf);
		case '[': return deserializeJsonArray(buf);
		case '"': return deserializeJsonString(buf);
		case 't':
		case 'f': return deserializeJsonBool(buf);
		case 'n': return deserializeJsonNull(buf);
		default:  return deserializeJsonNumber(buf);
	}
}

// -- cleanup

void freeJsonObject(jsonElement* elem) {
	if(!elem) return;

	// free children if present 
	if(elem->type == JSON_OBJECT
	|| elem->type == JSON_ARRAY) {
		freeJsonObject(elem->value.head);
	}

	// free next if present
	if(elem->next) freeJsonObject(elem->next);

	// free this
	free(elem);
}
