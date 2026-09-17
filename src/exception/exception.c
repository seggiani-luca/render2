#include "exception.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define ANSI_RESET    "\033[0m"
#define ANSI_YELLOW   "\033[33m"
#define ANSI_RED      "\033[31m"
#define ANSI_DARK_RED "\033[38;5;88m"

// -- logging

// gets string representation of event class
char* classToString(eventClass clas) {
	switch(clas) {
		case INFO:  return "info";
		case WARN:  return "warning";
		case ERROR: return "error";
		case FATAL: return "fatal";
		default:    return ""; 
	}
}

// gets color from event class
const char* classToColor(eventClass clas) {
	switch(clas) {
		case WARN:  return ANSI_YELLOW;
		case ERROR: return ANSI_RED;
		case FATAL: return ANSI_DARK_RED;
		default:    return ANSI_RESET;
	}
}

// gets string representation of event category 
char* categoryToString(eventCategory categ) {
	switch(categ) {
		case IO:     return "file";
		case JSON:   return "json";
		case SERIAL: return "serialization";
		case LISP:   return "script parse";
		case EXEC:   return "script exec.";
		case GUI:    return "win";
		case GL:     return "opengl";
		case SCENE:  return "scene";
		case GLSL:   return "glsl";
		case MEMORY: return "memory";
		case EXCEPT: return "exception";
		default:     return "unknown"; 
	}
}

void printEvent(event* ev) {
	printf("[%s%s%s] [%s]:\t%s\n",
		classToColor(ev->clas),
		classToString(ev->clas),
		ANSI_RESET,
		categoryToString(ev->categ),
		ev->mess);
}

void logEvent(eventClass clas, eventCategory categ, const char* fmt, ...) {
	// get current event
	event ev;

	// setup event
	ev.clas = clas;
	ev.categ = categ;

	// format message
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(ev.mess, EVENT_SIZE, fmt, ap);
	va_end(ap);

	// print event
	printEvent(&ev);
}

// -- allocation

void* xmalloc(size_t size) {
	void* ptr = malloc(size);

	if(!ptr && size != 0) {
		logEvent(FATAL, MEMORY, "Memory allocation failed via malloc()");
		exit(1);
	}

	return ptr;
}

void* xcalloc(size_t count, size_t size) {
	void* ptr = calloc(count, size);

	if(!ptr && count != 0 && size != 0) {
		logEvent(FATAL, MEMORY, "Memory allocation failed via calloc()");
		exit(1);
	}

	return ptr;
}

void* xrealloc(void* old, size_t size) {
	void* ptr = realloc(old, size);

	if(!ptr && size != 0) {
		logEvent(FATAL, MEMORY, "Memory allocation failed via realloc()");
		exit(1);
	}

	return ptr;
}

// -- exceptions

jmp_buf* cpPointer = NULL;
