#include "data/data.h"
#include "window/window.h"
#include "scene/scene.h"
#include "render/render.h"

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
	// free data tables
	freeTables();

	// free windows
	freeWindow(mainWin);
	freeWindow(inspectorWin);
	freeWindow(hierarchyWin);

	// terminate OpenGL
	freeGl();
}

// -- main

int main() {
	// create scene
	scene* mainScene = newScene("Main Scene");

	// create main window
	mainWin = newWindow(
		MAIN_WIDTH,
		MAIN_HEIGHT,
		"Main",
		makeRenderCallback(mainScene),
		1
	);

	// create editor windows
	createEditorWindows(mainScene);

	// init default scene
	initDefaultScene(mainScene);

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
