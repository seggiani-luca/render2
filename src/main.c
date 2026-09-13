#include "data/data.h"
#include "exception/exception.h"
#include "window/window.h"
#include "scene/scene.h"
#include "render/render.h"
#include <string.h>

// -- windows

// main engine window
window* mainWin = NULL;

// entity hierarchy
window* hierarchyWin = NULL;

// entity inspector
window* inspectorWin = NULL;

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
		loadIcon(WIN_DEFAULT_ICO),
		1
	);
	if(!mainWin) {
		logEvent(FATAL, GUI, "Couldn't create main window");	
		dumpEvents();

		freeScene(mainScene);
		return 1;
	}

	// create editor windows
	createEditorWindows(mainScene);
	if(!hierarchyWin || !inspectorWin) {
		logEvent(FATAL, GUI, "Couldn't create editor windows");	
		dumpEvents();

		freeScene(mainScene);
		cleanup();
		return 1;
	}
	centerWindows(hierarchyWin, mainWin, inspectorWin);
	
	// init default scene
	initDefaultScene(mainScene);
	
	// script init hook
	// TODO

	// main loop 
	for(;;) {
		// script update hook 
		// TODO

		// update windows
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
