#ifndef GUI_H
#define GUI_H

typedef struct entity entity; // forward entity
typedef struct field field;
#include "../window/window.h"

// -- rendering

// size of GUI queue
#define QUEUE_SIZ 512

// size of field entry box
#define FIELD_SIZ 64

// paths for GUI shader
#define GUI_VERT_PATH "dat/shader/gui_vert.glsl"
#define GUI_FRAG_PATH "dat/shader/gui_frag.glsl"

// path for GUI atlas
#define GUI_ATLAS_PATH "dat/texture/atlas.tga"
#define ATLAS_SIZ 256

// forward declaration of gui context
typedef struct guiContext guiContext;

#endif
