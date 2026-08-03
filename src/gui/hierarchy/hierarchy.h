#ifndef GUI_HIERARCHY_H
#define GUI_HIERARCHY_H

#include "../../scene/scene.h"
#include "../gui.h"

// -- sizing

// hierarchy size
#define HIERARCHY_WIDTH  400
#define HIERARCHY_HEIGHT 800

// width of hierarchy element
#define HIER_ELEM_WIDTH 160.0f

// add child window size
#define ADD_CHILD_WIDTH 400
#define ADD_CHILD_HEIGHT 160 

// offset for new child button
#define CHILD_OFF 50.0f 

// -- scenes

// gets a callback object for a scene hierarchy
renderCallback makeSceneCallback(scene* scn);

#endif
