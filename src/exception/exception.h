#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdlib.h>
#include <setjmp.h>

// -- logging

// number of events to keep logged
#define LOG_EVENTS 64

// max size of an event
#define EVENT_SIZE 256

// enum for event classes
typedef enum {
	INFO,
	WARN,
	ERROR,
	FATAL
} eventClass;

// sets verbosity up to event class
void setVerbose(eventClass clas);

// enum for event categories 
typedef enum {
	IO,     // file I/O
	JSON,   // json parsing
	SERIAL, // object serialization
	LISP,   // lisp script parsing
	EXEC,   // lisp script execution
	GUI,    // windows / GUI
	GL,     // OpenGL loading
	SCENE,  // scene management
	GLSL,   // GLSL shader compilation
	MEMORY, // memory allocation
	EXCEPT  // exceptions
} eventCategory;

// struct for events
typedef struct {
	// class of event
	eventClass clas;

	// category of event
	eventCategory categ;

	// message of event
	char mess[EVENT_SIZE];
} event;

// logs an event
void logEvent(eventClass clas, eventCategory categ, const char* fmt, ...);

// prints an event
void printEvent(event* ev);

// called by top level handler, dumps the received events in reverse order
void dumpEvents();

// -- allocation

// failure handling malloc
void* xmalloc(size_t size);

// failure handling calloc 
void* xcalloc(size_t count, size_t size);

// failure handling realloc 
void* xrealloc(void* old, size_t size);

// -- exceptions

// jump buffer pointer
extern jmp_buf* cpPointer;

// initializes exceptions
#define INIT_JUMPS                                       \
	jmp_buf cp;                                          \
	__attribute__((unused)) jmp_buf* prevCp = cpPointer; \
	cpPointer = &cp

// restores exceptions
#define RESTORE_JUMPS \
	(cpPointer = prevCp)

// try-catch blocks, automatically restores jumps
// WARNING - Don't return from the catch block or you'll leak state!
#define try                            \
	for (int _exc_once = 1; _exc_once; \
	     _exc_once = 0, RESTORE_JUMPS) \
	    if (setjmp(cp) == 0)
#define catch else

// throw clause
#define throw                                                     \
	{                                                             \
	    if (cpPointer) longjmp(*cpPointer, 1);                    \
	    else {                                                    \
	        logEvent(FATAL, EXCEPT, "Unhandled exception throw"); \
	        dumpEvents();                                         \
	        exit(1);                                              \
	    }                                                         \
	}

#endif
