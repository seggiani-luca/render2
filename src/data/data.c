#include "data.h"
#include "shader/shader.h"
#include "texture/texture.h"
#include "../window/window.h"
#include <stdlib.h>
#include <string.h>

// hook into main window
extern window* mainWin;

// -- data tables 

void printTable(const dataTable* table) {
	printf("%-25s %-25s %-25s %s\n", "Address", "Path", "References", "Data");

	// go through each table entry
	dataRef* ref = table->root;
	while(ref) {
		// print entry
		printf("%-25p %-25s %-25d ", ref->data, ref->path, ref->refCount);

		// print the data
		table->print(ref->data);
		printf("\n");

		ref = ref->next;
	}
}

dataRef* importData(const char* path, dataTable* table) {
	// move context to main
	GLFWwindow* old = glfwGetCurrentContext();
	glfwMakeContextCurrent(mainWin->gl);

	// query table by path
	dataRef* ref = table->root;
	while(ref) {
		// return if found
		if(strcmp(ref->path, path) == 0) {
			ref->refCount++;

			glfwMakeContextCurrent(old);
			return ref; 
		}

		ref = ref->next;
	}

	// walk table to end
	dataRef** cur = &table->root;
	while(*cur)cur = &(*cur)->next;
	
	// allocate entry
	dataRef* newRef = malloc(sizeof(dataRef));
	if(!newRef) {
		glfwMakeContextCurrent(old);
		return NULL;
	}

	// open file file
	FILE* file = fopen(path, "rb");
	if(file == NULL) {
		free(newRef);
		glfwMakeContextCurrent(old);
		return NULL;
	}

	// import data
	void* data = table->import(file);
	if(data == NULL) {
		free(newRef);
		glfwMakeContextCurrent(old);
		return NULL;
	}

	// close file
	fclose(file);

	// insert in table
	newRef->next = NULL;
	*cur = newRef;

	// copy data, setup reference
	strncpy(newRef->path, path, DAT_PATH_SIZ);
	newRef->path[DAT_PATH_SIZ - 1] = '\0';
	newRef->refCount = 1;
	newRef->data = data;

	// return data
	glfwMakeContextCurrent(old);
	return newRef; 
}

void freeData(void* dat, dataTable* table) {
	// only if data is not NULL
	if(dat == NULL) return;

	// locate in table
	dataRef** cur = &table->root;
	while(*cur) {
		// match by data
		if((*cur)->data == dat) {
			(*cur)->refCount--;

			// free on no refs
			if((*cur)->refCount == 0) {
				table->free((*cur)->data);
				dataRef* tmp = *cur;
				(*cur) = (*cur)->next;
				free(tmp);
			}

			return;
		}

		cur = &(*cur)->next;
	}
}

void freeTable(dataTable* table) {
	// delete each entry
	dataRef* cur = table->root;
	while(cur) {
		table->free(cur->data);

		dataRef* tmp = cur;
		cur = cur->next;
		free(tmp);
	}

	table->root = NULL;
}

void freeTables() {
	freeTable(&textureTable);
	freeTable(&shaderTable);
}
