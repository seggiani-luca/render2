#include "entity/entity.h"
#include "window/window.h"
#include "gui/inspector/inspector.h"
#include "gui/hierarchy/hierarchy.h"
#include "data/data.h"

// -- windows

// main engine window
window* mainWin;

// entity hierarchy
window* hierarchyWin;

// entity inspector
window* inspectorWin;

// -- utils

// cleans up after termination
void cleanup() {
	// free windows
	freeWindow(inspectorWin);
	freeWindow(hierarchyWin);

	// free data tables
	freeTables();

	// terminate OpenGL
	freeGl();
}

// -- main

int main() {
	// create scene
	scene* mainScene = newScene("Main Scene");
	for(int i = 0; i < 10; i++) {
		char entName[ENT_NAME_SIZ];
		sprintf(entName, "Entity %d", i);
		entity* ent = newEntity(entName);

		for(int j = 0; j < 3; j++) {
			char fldName[ENT_NAME_SIZ];
			sprintf(fldName, "Field %d", j);
			appendField(ent, intNew(fldName));
		}

		appendChild(&mainScene->root, ent);
	}
	
	// create hierarchy 
	hierarchyWin = newWindow(
		HIERARCHY_WIDTH,
		HIERARCHY_HEIGHT,
		"Hierarchy",
		makeSceneCallback(mainScene)
	);

	// create inspector
	inspectorWin = newWindow(
		INSPECTOR_WIDTH,
		INSPECTOR_HEIGHT,
		"Inspector",
		makeEntityCallback(NULL)
	);
	
	// update windows
	for(;;) {
		updateGl();
		if(!updateWindow(hierarchyWin)) break;
		if(!updateWindow(inspectorWin)) break;
	}

	freeScene(mainScene);

	// free all data
	cleanup();
}
