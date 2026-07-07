#ifndef GUI_HIERARCHY_H
#define GUI_HIERARCHY_H

#include "../../scene/scene.h"
#include "../gui.h"

// -- sizing

// hierarchy size
#define HIERARCHY_WIDTH 400
#define HIERARCHY_HEIGHT 800

// width of hierarchy element
#define HIER_ELEM_WIDTH 160.0f

// add field window size
#define ADD_CHILD_WIDTH 320
#define ADD_CHILD_HEIGHT 70

// offset for new field button
#define NEW_OFF 50.0f

// -- scenes

// gets a callback object for a scene hierarchy
renderCallback makeSceneCallback(scene* scn);

#endif
