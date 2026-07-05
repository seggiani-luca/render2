#ifndef GUI_INSPECTOR_H
#define GUI_INSPECTOR_H

#include "../gui.h"

#define INSPECTOR_WIDTH  320
#define INSPECTOR_HEIGHT 640
#define NEW_FIELD_HEIGHT 140

// -- field primitives

// renders an int field to GUI
void intFieldGui(const field* f, guiContext* ctx);

// renders a float field to GUI
void floatFieldGui(const field* f, guiContext* ctx);

// renders a string field to GUI
void stringFieldGui(const field* f, guiContext* ctx);

// -- entities

// gets a callback object for an entity inspector
renderCallback makeEntityCallback(entity* ent);

#endif
