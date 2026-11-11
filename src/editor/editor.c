#include "editor.h"
#include "../../lib/window/window.h"
#include "hierarchy/hierarchy.h"
#include "inspector/inspector.h"
#include "../exception/exception.h"

// -- windows

// hook into entity hierarchy
extern window* hierarchyWin;

// hook into entity inspector
extern window* inspectorWin;

void createEditorWindows(scene* scn) {
	// create hierarchy
	hierarchyWin = winNew(
		HIERARCHY_WIDTH,
		HIERARCHY_HEIGHT,
		"Hierarchy",
		makeSceneCallback(scn),
		NULL,
		WIN_DEPTH_OFF
	);
	if(!hierarchyWin) logEvent(ERROR, GUI, "Couldn't create hierarchy window");

	// create inspector
	inspectorWin = winNew(
		INSPECTOR_WIDTH,
		INSPECTOR_HEIGHT,
		"Inspector",
		makeEntityCallback(NULL),
		NULL,
		WIN_DEPTH_OFF
	);
	if(!inspectorWin) logEvent(ERROR, GUI, "Couldn't create inspector window");
}
