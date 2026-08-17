#include "data/data.h"
#include "window/window.h"
#include "scene/scene.h"
#include "render/render.h"
#include "gui/hierarchy/hierarchy.h"
#include "gui/inspector/inspector.h"

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
	freeWindow(mainWin);
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
	scene* mainScene = newDefaultScene("Main Scene");

	// create main window
	mainWin = newWindow(
		MAIN_WIDTH,
		MAIN_HEIGHT,
		"Main",
		makeRenderCallback(mainScene),
		1
	);

	// create hierarchy
	hierarchyWin = newWindow(
		HIERARCHY_WIDTH,
		HIERARCHY_HEIGHT,
		"Hierarchy",
		makeSceneCallback(mainScene),
		0
	);

	// create inspector
	inspectorWin = newWindow(
		INSPECTOR_WIDTH,
		INSPECTOR_HEIGHT,
		"Inspector",
		makeEntityCallback(NULL),
		0
	);

	// update windows
	for(;;) {
		updateGl();
		if(!updateWindow(mainWin)) break;
		if(!updateWindow(hierarchyWin)) break;
		if(!updateWindow(inspectorWin)) break;
	}

	// free scene
	freeScene(mainScene);

	// free all data
	cleanup();
}
