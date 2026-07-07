#ifndef GUI_INSPECTOR_H
#define GUI_INSPECTOR_H

#include "../../scene/entity/entity.h"
#include "../gui.h"

// -- sizing

// inspector size
#define INSPECTOR_WIDTH 400
#define INSPECTOR_HEIGHT 800

// add field window size
#define ADD_FIELD_WIDTH 320
#define ADD_FIELD_HEIGHT 130

// offset for new field button
#define NEW_OFF 50.0f

// -- field primitives

// renders an int field to GUI
int intFieldGui(const field* f, guiContext* ctx);

// renders a float field to GUI
int floatFieldGui(const field* f, guiContext* ctx);

// renders a string field to GUI
int stringFieldGui(const field* f, guiContext* ctx);

// renders a 2D vector field to GUI
int float2FieldGui(const field* f, guiContext* ctx);

// renders a 3D vector field to GUI
int float3FieldGui(const field* f, guiContext* ctx);

// renders a 4D vector field to GUI
int float4FieldGui(const field* f, guiContext* ctx);

// -- entities

// gets a callback object for an entity inspector
renderCallback makeEntityCallback(entity* ent);

// changes entity callback
void changeEntityCallback(window* win, entity* ent);

// gets entity of entity callback
entity* getEntityCallback(window* win);

#endif
