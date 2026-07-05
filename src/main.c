#include "entity/entity.h"
#include "window/window.h"
#include "gui/inspector/inspector.h"
#include "data/data.h"

// -- windows

window* mainWin;
window* inspectorWin;
window* hierarchyWin;

// -- main

int main() {
	// create example entity
	entity* ent = newEntity("Enterprise");
	appendField(ent, stringNew(
		"Captain",
		"Kirk"
	));
	appendField(ent, floatNew(
		"Weight",
		10.5
	));
	appendField(ent, intNew(
		"Missiles",
		25
	));

	// create inspector
	inspectorWin = newWindow(
		320,
		640,
		"Inspector",
		makeEntityCallback(ent)
	);
	
	// update windows
	for(;;) {
		updateGl();
		if(!updateWindow(inspectorWin)) break;
	}

	// free example entity
	freeEntity(ent);

	// free windows
	freeWindow(inspectorWin);

	// free data tables
	freeTables();

	// terminate OpenGL
	freeGl();
}
