#ifndef GUI_INSPECTOR_H
#define GUI_INSPECTOR_H

#include "../gui.h"
#include "../../entity/entity.h"

// -- sizing

// inspector size
#define INSPECTOR_WIDTH  400
#define INSPECTOR_HEIGHT 800

// add field window size
#define ADD_FIELD_WIDTH  320
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

// -- entities

// gets a callback object for an entity inspector
renderCallback makeEntityCallback(entity* ent);

// changes entity callback
void changeEntityCallback(window* win, entity* ent);

// gets entity of entity callback
entity* getEntityCallback(window* win);

#endif
