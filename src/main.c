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
	for(int i = 0; i < 40; i++) {
		appendChild(&mainScene->root, newEntity("Entity"));
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
