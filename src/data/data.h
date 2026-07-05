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
typedef dataRef* dataTable;

// data table and handlers declaration macro
#define DATA_TABLE_DECL(type)             \
    void type##PrintTable();              \
    type* type##Import(const char* path); \
    void type##Free(type* obj);           \
    void type##FreeTable();

// data table and handlers implementation macro
#define DATA_TABLE_IMPL(type)                                                 \
    dataTable type##Table = NULL;                                             \
                                                                              \
    void type##PrintTable() {                                                 \
        printTable(type##Table, (void(*)(void*))type##Print);                 \
    }                                                                         \
                                                                              \
    type* type##Import(const char* path) {                                    \
        return importData(                                                    \
            path,                                                             \
            &type##Table,                                                     \
            (void* (*)(FILE*)) type##_import                                  \
            );                                                                \
    }                                                                         \
                                                                              \
    void type##Free(type* data) {                                             \
        freeData(data, &type##Table, (void(*)(void*))type##_free);            \
    }                                                                         \
                                                                              \
    void type##FreeTable() {                                                  \
        freeTable(&type##Table, (void(*)(void*))type##_free);                 \
    }

// debug prints a data table
void printTable(dataTable table, void (*printData)(void*));

// queries data table (importing on table miss) and returns value
void* importData(const char* path, dataTable* table, void* (*d_import)(FILE*));

// frees an entry of a data table 
void freeData(void* data, dataTable* table, void (*d_free)(void*));

// frees a data table 
void freeTable(dataTable* table, void (*d_free)(void*));

// frees all data tables
void freeTables();

#endif
