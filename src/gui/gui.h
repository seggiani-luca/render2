#ifndef GUI_H
#define GUI_H

#include "../window/window.h"

// -- rendering

// paths for GUI shaders
#define GUI_VERT_PATH "dat/shader/gui_vert.glsl"
#define GUI_FRAG_PATH "dat/shader/gui_frag.glsl"

// path for GUI atlas
#define GUI_ATLAS_PATH "dat/texture/gui_atlas.tga"
#define ATLAS_DIM 256
#define GUI_SCALE 1.0f
#define ATLAS_SIZ (ATLAS_DIM * GUI_SCALE)

// forward declaration of GUI context
typedef struct guiContext guiContext;

// forward declaration of scene
typedef struct scene scene;

// creates editor windows
void createEditorWindows(scene* scn);

#endif
