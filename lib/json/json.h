/*
 * Minimal JSON serialization / deserialization library. Only supports object
 * serialization / deserialization for now.
 * Example usage:
 *
 * char* ptr = buf;
 *
 * // provide your own allocator
 * jsonElement* obj = deserializeJsonObject(deserialAllocator, &ptr);
 *
 * if(!obj) {
 *     // free allocator parsing
 *     freeAllocator();
 *     free(buf);
 *     return;
 * }
 *
 * // get a string from an object
 * const char* name = getJsonString(getJsonAtKey(obj, "name"));
 *
 * // iterate over an array
 * jsonElement* cur = getJsonHead(getJsonAtKey(obj, "fields"));
 * while(cur) {
 *     // process element
 *     cur = getJsonNext(cur);
 * }
 *
 * freeJsonObject(free, obj);
 *
 * 2026 - Luca Seggiani
 */

#ifndef JSON_H
#define JSON_H

#include <stdio.h>

// -- setup

/**
 * Allocator callback used to allocate JSON elements.
 * Receives the requested allocation size and returns the allocated memory,
 * or NULL on failure.
 *
 * This can be used to provide a custom allocation pool for object
 * deserialization, which the user can then free on its own terms if the
 * library fails.
 * Note that the library doesn't free its own memory when it's deserializing
 * very deep nested objects and it fails.
 */
typedef void* (*jsonAllocator)(size_t);

/**
 * Deallocator callback used to release JSON elements (dual of the allocator
 * above).
 */
typedef void (*jsonDeallocator)(void*);

// -- definitions

/**
 * Type of JSON element.
 */
typedef enum {
	JSON_STRING,
	JSON_NUMBER,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BOOLEAN,
	JSON_NULL
} jsonType;

/**
 * JSON element definition.
 */
typedef struct jsonElement {
	/**
	 * Key associated with the element when it belongs to an object.
	 * NULL for array elements and root elements.
	 */
	const char* key;

	/**
	 * Type of the stored value.
	 */
	jsonType type;

	/**
	 * Payload of the element, interpreted according to its type.
	 */
	union {
		/**
		 * Pointer to the string value.
		 */
		const char* string;

		/**
		 * Floating-point number value.
		 */
		float number;

		/**
		 * Integer boolean value (0 or nonzero).
		 */
		int boolean;

		/**
		 * First child of an object or array.
		 */
		struct jsonElement* head;
	} value;

	/**
	 * Pointer to the next element in the containing object or array.
	 */
	struct jsonElement* next;
} jsonElement;

// -- getting

/**
 * Returns the value associated with a key in an object.
 *
 * @param elem object to search
 * @param key  key to look for
 * @return     matching element, or NULL if not found
 */
jsonElement* getJsonAtKey(const jsonElement* elem, const char* key);

/**
 * Returns the first child of an object or array.
 *
 * @param elem object or array to inspect
 * @return     first child, or NULL if empty
 */
jsonElement* getJsonHead(const jsonElement* elem);

/**
 * Returns the next element in an object's or array's child list.
 *
 * @param elem current element
 * @return     next element, or NULL if there is none
 */
jsonElement* getJsonNext(const jsonElement* elem);

/**
 * Returns the string value of an element.
 *
 * @param elem element to inspect
 * @return     string value, or NULL if unavailable
 */
const char* getJsonString(const jsonElement* elem);

/**
 * Returns the floating-point value of an element.
 *
 * @param elem element to inspect
 * @return     number value
 */
float getJsonNumber(const jsonElement* elem);

/**
 * Returns the boolean value of an element.
 *
 * @param elem element to inspect
 * @return     boolean value
 */
int getJsonBoolean(const jsonElement* elem);

/**
 * Checks whether an element represents JSON null.
 *
 * @param elem element to inspect
 * @return     nonzero if the element is JSON null, zero otherwise
 */
int isJsonNull(const jsonElement* elem);

// -- setting

/**
 * Adds an element to an object under the specified key.
 *
 * @param elem object to modify
 * @param n    element to add
 * @param key  key associated with the new element
 * @return     modified object
 */
jsonElement* addJsonAtKey(jsonElement* elem, jsonElement* n, const char* key);

/**
 * Appends an element to the end of an array.
 *
 * @param elem array to modify
 * @param n    element to append
 * @return     modified array
 */
jsonElement* addJsonAtTail(jsonElement* elem, jsonElement* n);

/**
 * Creates a JSON string element.
 *
 * @param a   allocator used for the element
 * @param val string value to store
 * @return    newly allocated element
 */
jsonElement* newJsonString(jsonAllocator a, const char* val);

/**
 * Creates a JSON number element.
 *
 * @param a   allocator used for the element
 * @param val floating-point value to store
 * @return    newly allocated element
 */
jsonElement* newJsonNumber(jsonAllocator a, float val);

/**
 * Creates an empty JSON object.
 *
 * @param a allocator used for the element
 * @return  newly allocated element
 */
jsonElement* newJsonObject(jsonAllocator a);

/**
 * Creates an empty JSON array.
 *
 * @param a allocator used for the element
 * @return  newly allocated element
 */
jsonElement* newJsonArray(jsonAllocator a);

/**
 * Creates a JSON boolean element.
 *
 * @param a   allocator used for the element
 * @param val boolean value to store
 * @return    newly allocated element
 */
jsonElement* newJsonBoolean(jsonAllocator a, int val);

/**
 * Creates a JSON null element.
 *
 * @param a allocator used for the element
 * @return  newly allocated element
 */
jsonElement* newJsonNull(jsonAllocator a);

// -- serialization / deserialization

/**
 * Serializes a JSON element tree to a file.
 *
 * @param file output file stream
 * @param elem root element to serialize
 */
void serializeJsonObject(FILE* file, jsonElement* elem);

/**
 * Parses a JSON document from a mutable character buffer.
 *
 * The parser may modify the input buffer. String values reference the original
 * buffer, which must remain valid while those values are in use.
 *
 * @param a   allocator used for JSON elements
 * @param buf pointer to the input buffer pointer
 * @return    parsed root element, or NULL on failure
 */
jsonElement* deserializeJsonObject(jsonAllocator a, char** buf);

/**
 * Recursively frees a JSON element tree.
 *
 * String storage is not freed separately.
 *
 * @param d    deallocator used for JSON elements
 * @param elem root element to free
 */
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
