#include "gui.h"
#include "hierarchy/hierarchy.h"
#include "inspector/inspector.h"

// hook into entity hierarchy
extern window* hierarchyWin;

// hook into entity inspector
extern window* inspectorWin;

void createEditorWindows(scene* scn) {
	// create hierarchy
	hierarchyWin = newWindow(
		HIERARCHY_WIDTH,
		HIERARCHY_HEIGHT,
		"Hierarchy",
		makeSceneCallback(scn),
		loadIcon(WIN_HIERCHY_ICO),
		0
	);

	// create inspector
	inspectorWin = newWindow(
		INSPECTOR_WIDTH,
		INSPECTOR_HEIGHT,
		"Inspector",
		makeEntityCallback(NULL),
		loadIcon(WIN_INSPECT_ICO),
		0
	);
}
