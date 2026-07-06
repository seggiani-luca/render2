#include "entity/entity.h"
#include "window/window.h"
#include "gui/inspector/inspector.h"
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

	// free data tables
	freeTables();

	// terminate OpenGL
	freeGl();
}

// -- main

int main() {
	entity* ent = newEntity("Entity");
	for(int i = 0; i < 10; i++) {
		char name[ENT_NAME_SIZ];
		sprintf(name, "Field %d", i);
		appendField(ent, intNew(name));
	}

	// create inspector
	inspectorWin = newWindow(
		INSPECTOR_WIDTH,
		INSPECTOR_HEIGHT,
		"Inspector",
		makeEntityCallback(ent)
	);
	
	// update windows
	for(;;) {
		updateGl();
		if(!updateWindow(inspectorWin)) break;
	}

	freeEntity(ent);

	// free all data
	cleanup();
}
