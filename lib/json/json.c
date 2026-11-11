#include "json.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

// -- utils

// JSON exception handling
static jmp_buf* jsonCpPointer = NULL;

// throws to return to previous jump pointer
#define JSON_THROW()                    \
	do {                                \
	    if(jsonCpPointer)               \
		    longjmp(*jsonCpPointer, 1); \
	    abort();                        \
	} while(0)

// current error message
static const char* strError = NULL;

static int isWhitespace(char c) {
	return c == '\n'
	    || c == '\r'
	    || c == '\t'
	    || c == ' ';
}

static int isDigit(char* buf) {
	// first check this digit
	char c0 = *buf;
	if(c0 >= '0' && c0 <= '9') return 1;

	// then check if positive/negative
	char c1 = *(buf + 1);
	if(c0 == '+' || c0 == '-')
		if(c1 >= '0' && c1 <= '9') return 1;

	return 0;
}

static void eatWhitespace(char** buf) {
	while(buf && *buf && isWhitespace(**buf)) {
		(*buf)++;
	}
}

static void expect(char** buf, const char* key) {
	int len = strlen(key);
	if(strncmp(*buf, key, len)) {
		strError = "Unexpected token";
		JSON_THROW();	
	} 
	
	*buf += len;
}

static int consume(char** buf, const char* key) {
	int len = strlen(key);
	if(strncmp(*buf, key, len)) return 0;
	
	*buf += len;
	return 1;
}

static const char* readString(char** buf) {
	// begin string
	expect(buf, "\"");

	// find end of string
	char* next = strchr(*buf, '\"');
	if(next == NULL) {
		strError = "Non terminated string";
		JSON_THROW();	
	} 

	// get return
	const char* ret = *buf;
	*next = '\0';

	// advance
	*buf = next + 1;

	return ret;
}

// inserts tabs to reach given depth
static void ftabulate(FILE* file, int depth) {
	for(int i = 0; i < depth; i++) fprintf(file, "\t");
}

// prints a string to a file 
static void fprintString(FILE* file, const char* str) {
	fprintf(file, "\"%s\"", str);
}

// -- getting 

jsonElement* getJsonAtKey(const jsonElement* elem, const char* key) {
	if(!elem) {
		strError = "No element to search for key in";
		return NULL;
	}
	if(elem->type != JSON_OBJECT) {
		strError = "Can't look for key in non-object element";
		return NULL;
	}

	// go through all children
	jsonElement* cur = elem->value.head;
	while(cur) {
		if(strcmp(cur->key, key) == 0) return cur;
		cur = cur->next;
	}

	return NULL;
}

jsonElement* getJsonHead(const jsonElement* elem) {
	if(!elem) {
		strError = "No element to get head of";
		return NULL;
	}
	if(elem->type != JSON_OBJECT && elem->type != JSON_ARRAY) {
		strError = "Can't get head of non-iterable element";
		return NULL;
	}

	return elem->value.head;
}

jsonElement* getJsonNext(const jsonElement* elem) {
	if(!elem) {
		strError = "No element to get next of";
		return NULL;
	}
	
	return elem->next;
}

const char* getJsonString(const jsonElement* elem) {
	if(!elem) {
		strError = "No element to get string of";
		return NULL;
	}
	if(elem->type != JSON_STRING) {
		strError = "Can't get string of non-string element";
		return NULL;
	}

	return elem->value.string;
}

float getJsonNumber(const jsonElement* elem) {
	if(!elem) {
		strError = "No element to get number of, defaulting to 0";
		return 0.0f; 
	}
	if(elem->type != JSON_NUMBER) {
		strError = "Can't get number of non-number element, defaulting to 0";
		return 0.0f;
	}
	
	return elem->value.number;
}

int getJsonBoolean(const jsonElement* elem) {
	if(!elem) {
		strError = "No element to get boolean of, defaulting to false";
		return 0; 
	}
	if(elem->type != JSON_BOOLEAN) {
		strError = "Can't get boolean of non-boolean element, defaulting to false";
		return 0; 
	}	
	return elem->value.boolean;
}

int isJsonNull(const jsonElement* elem) {
	if(!elem) {
		strError = "No element to check NULLness of, defaulting to NULL";
		return 1; 
	}
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

jsonElement* newJsonString(jsonAllocator a, const char* val) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		return NULL;
	}

	// setup element 
	new->type = JSON_STRING;
	new->next = NULL;
	new->value.string = val;

	return new;
}

jsonElement* newJsonNumber(jsonAllocator a, float val) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		return NULL;
	}
	
	// setup element 
	new->type = JSON_NUMBER;
	new->next = NULL;
	new->value.number = val;

	return new;
}

jsonElement* newJsonObject(jsonAllocator a) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		return NULL;
	}
	
	// setup element 
	new->type = JSON_OBJECT;
	new->next = NULL;
	new->value.head = NULL;

	return new;
}

jsonElement* newJsonArray(jsonAllocator a) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		return NULL;
	}
	
	// setup element 
	new->type = JSON_ARRAY;
	new->next = NULL;
	new->value.head = NULL;

	return new;
}

jsonElement* newJsonBoolean(jsonAllocator a, int val) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		return NULL;
	}
	
	// setup element 
	new->type = JSON_BOOLEAN;
	new->next = NULL;
	new->value.boolean = val;

	return new;
}

jsonElement* newJsonNull(jsonAllocator a) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		return NULL;
	}
	
	// setup element 
	new->type = JSON_NULL;
	new->next = NULL;

	return new;
}

// -- serialization

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
		ftabulate(file, depth + 1);

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
	ftabulate(file, depth);
	fprintf(file, "}");
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
		ftabulate(file, depth + 1);
		
		// serialize value
		serializeJsonValue(file, cur, depth + 1);

		// comma terminate
		if(cur->next) fprintf(file, ",");
		fprintf(file, "\n");

		cur = cur->next;
	}

	// close array
	ftabulate(file, depth);
	fprintf(file, "]");	
}

// serializes a JSON string to a file
void serializeJsonString(FILE* file, jsonElement* elem) {
	fprintString(file, elem->value.string);
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
	fprintString(file, elem->key);
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

void serializeJsonObject(FILE* file, jsonElement* elem) {
	doSerializeJsonObject(file, elem, 0);
}

// -- deserialization 

// forward declarations for JSON object parsing
const char* deserializeJsonKey(jsonAllocator a, char** buf);
jsonElement* deserializeJsonValue(jsonAllocator a, char** buf);

jsonElement* doDeserializeJsonObject(jsonAllocator a, char** buf) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		JSON_THROW();
	}
	
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
		const char* key = deserializeJsonKey(a, buf);

		// key-value separator
		eatWhitespace(buf);
		expect(buf, ":");
		eatWhitespace(buf);

		// get element value
		jsonElement* elem = deserializeJsonValue(a, buf);
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
jsonElement* deserializeJsonArray(jsonAllocator a, char** buf) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		JSON_THROW();
	}
	
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
		jsonElement* elem = deserializeJsonValue(a, buf);
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
jsonElement* deserializeJsonString(jsonAllocator a, char** buf) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		JSON_THROW();
	}
	
	// setup element 
	new->type = JSON_STRING;
	new->next = NULL;

	// get string
	const char* str = readString(buf);
	new->value.string = str;

	return new;
}

// parses a JSON boolean from a buffer
jsonElement* deserializeJsonBool(jsonAllocator a, char** buf) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		JSON_THROW();
	}
	
	// setup element 
	new->type = JSON_BOOLEAN;
	new->next = NULL;

	// get boolean
	if(consume(buf, "true")) {
		new->value.boolean = 1;
	} else if(consume(buf, "false")) {
		new->value.boolean = 0;
	} else {
		strError = "Invalid JSON boolean value";
		JSON_THROW();
	}
	
	return new;
}

// parses a JSON null from a buffer
jsonElement* deserializeJsonNull(jsonAllocator a, char** buf) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		JSON_THROW();
	}
	
	// setup element 
	new->type = JSON_NULL;
	new->next = NULL;

	// get NULL
	expect(buf, "null");

	return new;
}

// parses a JSON numbe from a buffer
jsonElement* deserializeJsonNumber(jsonAllocator a, char** buf) {
	// allocate element
	jsonElement* new = a(sizeof(jsonElement));
	if(!new) {
		strError = "Memory allocation failed";
		JSON_THROW();
	}
	
	// setup element 
	new->type = JSON_NUMBER;
	new->next = NULL;

	// get number
	char *end;
	new->value.number = strtof(*buf, &end);

	// validate trash reads
	if (end == *buf) {
		strError = "Invalid JSON number value";
		JSON_THROW();
	}

	// advance 
	*buf = end;

	return new;
}

// parses a JSON key from a buffer
const char* deserializeJsonKey(jsonAllocator a __attribute__((unused)), char** buf) {
	return readString(buf);	
}

// parses a JSON value from a buffer
jsonElement* deserializeJsonValue(jsonAllocator a, char** buf) {
	eatWhitespace(buf);

	// distinguish value type
	switch(**buf) {
		case '{': return doDeserializeJsonObject(a, buf);
		case '[': return deserializeJsonArray(a, buf);
		case '"': return deserializeJsonString(a, buf);
		case 't':
		case 'f': return deserializeJsonBool(a, buf);
		case 'n': return deserializeJsonNull(a, buf);
		default:  return deserializeJsonNumber(a, buf);
	}
}

jsonElement* deserializeJsonObject(jsonAllocator a, char** buf) {
	// save previous exception handler
	strError = NULL;
	jmp_buf* prev = jsonCpPointer;

	// install local exception handler
	jmp_buf env;
	jsonCpPointer = &env;

	// attempt deserialization
	if(setjmp(env) == 0) {
		jsonElement* result = doDeserializeJsonObject(a, buf);

		// restore previous exception handler
		jsonCpPointer = prev;

		return result;
	}

	// restore previous exception handler after exception
	jsonCpPointer = prev;

	return NULL;
}

// -- cleanup

void freeJsonObject(jsonDeallocator d, jsonElement* elem) {
	if(!elem) return;

	// free children if present 
	if(elem->type == JSON_OBJECT
	|| elem->type == JSON_ARRAY) {
		freeJsonObject(d, elem->value.head);
	}

	// free next if present
	if(elem->next) freeJsonObject(d, elem->next);

	// free this
	d(elem);
}

// -- errors

// reports JSON errors
const char* jsonError() {
	const char* prev = strError;
	strError = NULL;
	return prev;
}
