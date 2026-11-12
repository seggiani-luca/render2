#include "exception/exception.h"
#include "data/data.h"
#include "input/input.h"
#include "time/time.h"
#include "scene/scene.h"
#include "editor/editor.h"
#include "../lib/window/window.h"

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
	winFree(inspectorWin);
	winFree(hierarchyWin);
	winFree(mainWin);
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
	winMove(a, x, ya);
	x += a->fbWidth + gap;
	winMove(b, x, yb);
	x += b->fbWidth + gap;
	winMove(c, x, yc);
}

// -- main

int main() {
	// create scene
	scene* mainScene = newScene("Main Scene");
	
	// create main window
	mainWin = winNew(
		MAIN_WIDTH,
		MAIN_HEIGHT,
		"Main",
		makeRenderCallback(mainScene),
		NULL,
		1
	);
	if(!mainWin) {
		logEvent(ERROR, GUI, winError());
		logEvent(FATAL, GUI, "Couldn't create main window");
		freeScene(mainScene);
		return 1;
	}

	// attach input listeners
	attachInput(mainWin->gl);

	// create editor windows
	createEditorWindows(mainScene);
	if(!hierarchyWin || !inspectorWin) {
		logEvent(ERROR, GUI, winError());
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
		scriptsUpdate(&mainScene->root, deltaTime());
	
		// reset input
		inputReset();

		// update windows
		if(!winUpdate(mainWin))      break;
		if(!winUpdate(hierarchyWin)) break;
		if(!winUpdate(inspectorWin)) break;
	}

	// free scene
	freeScene(mainScene);

	// free all data
	cleanup();
}
