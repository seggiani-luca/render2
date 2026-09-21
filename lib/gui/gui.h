#ifndef GUI_H
#define GUI_H

#include "../window/window.h"

// -- rendering

// paths for GUI shaders
#define GUI_VERT_PATH "dat/shader/gui_vert.glsl"
#define GUI_FRAG_PATH "dat/shader/gui_frag.glsl"

// path for GUI atlas
#define GUI_ATLAS_PATH "dat/texture/gui_atlas.tga"

// structure of GUI atlas:
// row 0-2: ASCII character set in standard order
// row 3: background colors:
//        - Absolute background (pitch black)
//        - Dark background (controls, fields)
//        - Light background (panels)
//        - Dark foreground (borders)
//        - Light foreground (text, icons)
// row 4-15: icons

// GUI scaling 
#define ATLAS_DIVISION 16                      // number of elements on axis 
#define ATLAS_DIM      1024                    // pixel size of atlas side 
#define GUI_SCALE      0.25f                   // scaling of atlas on screen
#define ATLAS_SIZ      (ATLAS_DIM * GUI_SCALE) // calculated atlas size

// forward declaration of GUI context
typedef struct guiContext guiContext;

#endif
