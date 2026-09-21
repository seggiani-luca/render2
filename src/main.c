#include "exception/exception.h"
#include "window/window.h"
#include "editor/editor.h"
#include "scene/scene.h"
#include "script/script.h"
#include "render/render.h"

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

void centerWindows(window* a, window* b, window* c) {
	// get monitor dimensions
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	int screenX, screenY, screenW, screenH;
	glfwGetMonitorWorkarea(
		monitor,
		&screenX, &screenY,
		&screenW, &screenH
	);

	// calculate total width 
	int gap = 10;
	int totalW =
		a->fbWidth +
		b->fbWidth +
		c->fbWidth +
		gap * 2;

	// get all y coordinates
	int ya = screenY + (screenH - a->fbHeight) / 2;
	int yb = screenY + (screenH - b->fbHeight) / 2;
	int yc = screenY + (screenH - c->fbHeight) / 2;

	// get starting x coordinate
	int x = screenX + (screenW - totalW) / 2;

	// move windows
	moveWindow(a, x, ya);
	x += a->fbWidth + gap;
	moveWindow(b, x, yb);
	x += b->fbWidth + gap;
	moveWindow(c, x, yc);
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
		freeScene(mainScene);
		return 1;
	}

	// create editor windows
	createEditorWindows(mainScene);
	if(!hierarchyWin || !inspectorWin) {
		logEvent(FATAL, GUI, "Couldn't create editor windows");	
		freeScene(mainScene);
		cleanup();
		return 1;
	}
	centerWindows(hierarchyWin, mainWin, inspectorWin);
	
	// init default scene
	initDefaultScene(mainScene);
	
	// script init hook
	scriptsStart(&mainScene->root);

	// main loop 
	for(;;) {
		// script update hook 
		scriptsUpdate(&mainScene->root);

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
