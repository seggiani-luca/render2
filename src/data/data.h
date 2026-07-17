#ifndef DATA_H
#define DATA_H

#include <stdio.h>

// -- data references

// data reference path size
#define DAT_PATH_SIZ 64

// data object name size
#define DAT_NAME_SIZ 64

// data reference
struct dataRef {
	// path of loaded data
	char path[DAT_PATH_SIZ];

	// pointer to data
	void* data;

	// reference counter
	int refCount;

	// next data reference in data table
	struct dataRef* next;
};
typedef struct dataRef dataRef;

// -- data tables

// data table typedef
typedef struct {
	// root of table
	dataRef* root;
	
	// print method
	void (*print)(void*);

	// import method
	void* (*import)(FILE*);

	// free method
	void (*free)(void*);
} dataTable;

// data table and handlers declaration macro
#define DATA_TABLE_DECL(type)                    \
	    dataRef* type##Import(const char* path); \
	    void type##Free(type* dat);              \
	    extern dataTable type##Table;

// data table and handlers implementation macro
#define DATA_TABLE_IMPL(type)                  \
	dataTable type##Table = {                  \
	    .root   = NULL,                        \
	    .print  = type##Print,                 \
	    .import = type##_import,               \
		.free   = type##_free                  \
	};                                         \
	                                           \
	dataRef* type##Import(const char* path) {  \
	    return importData(path, &type##Table); \
	}                                          \
	                                           \
	void type##Free(type* dat) {               \
	    freeData(dat, &type##Table);           \
	}

// debug prints a data table
void printTable(dataTable table);

// queries data table (importing on table miss) and returns value
dataRef* importData(const char* path, dataTable* table);

// frees an entry of a data table
void freeData(void* dat, dataTable* table);

// frees a data table
void freeTable(dataTable* table);

// frees all data tables
void freeTables();

#endif
