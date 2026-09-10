#include "data/data.h"
#include "script/lisp/lisp.h"
#include "window/window.h"
#include "scene/scene.h"
#include "render/render.h"
#include "data/script/script.h"

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

#define TEST_SCRIPT "dat/script/test.scm"
int main() {
	script* scr = scriptImport(TEST_SCRIPT)->data;
	printEnvironment(scr->env);
	scriptFree(scr);
	return 0;

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

	// create editor windows
	createEditorWindows(mainScene);

	// center windows
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
