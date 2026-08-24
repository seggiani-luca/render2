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

// nicely centers windows on the screen
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

	// get all ys
	int ya = screenY + (screenH - a->fbHeight) / 2;
	int yb = screenY + (screenH - b->fbHeight) / 2;
	int yc = screenY + (screenH - c->fbHeight) / 2;

	// get starting x 
	int x = screenX + (screenW - totalW) / 2;

	// move windows
	moveWindow(a, x, ya);
	x += a->fbWidth + gap;

	moveWindow(b, x, yb);
	x += b->fbWidth + gap;

	moveWindow(c, x, yc);
}

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

	// center windows
	centerWindows(hierarchyWin, mainWin, inspectorWin);

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
