#include "data.h"
#include "shader/shader.h"
#include "texture/texture.h"
#include <stdlib.h>
#include <string.h>

void printTable(dataTable table, void (*printData)(void*)) {
	printf("%-25s %-25s %-25s %s\n", 
		"Address", 
		"Path", 
		"References", 
		"Data"
	);

	// go through each table entry
	dataRef* ref = table;
	while(ref) {
		// print entry
		printf("%-25p %-25s %-25d ",
			ref->data,
			ref->path,
			ref->refCount
		);

		// print the data
		printData(ref->data);
		printf("\n");

		ref = ref->next;
	}
}

void* importData(
	const char* path,
	dataTable* table,
	void* (*d_import)(FILE*)
) {
	// query table by path
	dataRef* ref = *table;
	while(ref) {
		// return if found
		if(strcmp(ref->path, path) == 0) {
			ref->refCount++;	
			return ref->data; 
		}

		ref = ref->next;
	}

	// allocate entry
	dataRef** cur = table;
	while(*cur) cur = &(*cur)->next;
	dataRef* newRef = malloc(sizeof(dataRef));
	if(!newRef) return NULL;
	
	// import data from file
	FILE* file = fopen(path, "r");
	if(file == NULL) {
		free(newRef);
		return NULL;
	}
	void* data = d_import(file);
	fclose(file);
	if(data == NULL) {
		free(newRef);
		return NULL;
	}

	// insert in table
	newRef->next = NULL;
	*cur = newRef;

	// copy data
	strncpy(newRef->path, path, DAT_PATH_SIZ);
	newRef->path[DAT_PATH_SIZ - 1] = '\0';
	newRef->refCount = 1;
	newRef->data = data;
	
	// return data
	return data;
}

void freeData(void* data, dataTable* table, void (*d_free)(void*)) {
	// only if data is not NULL
	if(data == NULL) return;
	
	// locate in table by data
	dataRef** cur = table;
	while(*cur) {
		if((*cur)->data == data) {
			(*cur)->refCount--;
			if((*cur)->refCount == 0) {
				d_free(data);
				dataRef* tmp = *cur;
				*cur = (*cur)->next;
				free(tmp);
			}

			return;
		}

		cur = &(*cur)->next;
	}
}

void freeTable(dataTable* table, void (*d_free)(void*)) {
	// delete each entry 
	dataRef* cur = *table;
	while(cur) {
		d_free(cur->data);
	
		dataRef* tmp = cur;
		cur = cur->next;
		free(tmp);
	}

	*table = NULL;
}

void freeTables() {
	printf("%-55s", "Freeing data tables ...");
	
	shaderFreeTable();
	textureFreeTable();
	
	printf("Success\n");
}
