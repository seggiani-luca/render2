#ifndef EDITOR_SELECTOR_H
#define EDITOR_SELECTOR_H

#include "../../gui/gui.h"
#include "../../gui/widget/widget.h"
#include "../../data/includes.h"

// -- constants

// data and shader selector size
#define DATASEL_WIDTH 600
#define DATASEL_HEIGHT 400

// path selector size
#define PATHSEL_WIDTH 500
#define PATHSEL_HEIGHT 500

// offset for import button
#define IMPORT_OFF 520.0f

// offset for path button
#define PATH_OFF 50.0f 

// -- selectors

// selects data for arbitrary table
int dataGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	dataRef** ref,
	dataTable* tab
);

// specializes dataGui for a certain data table
#define SELECTOR_GUI(data)                                              \
	static inline int data##Gui(                                        \
	    guiContext* ctx,                                                \
	    guiLayerId layId,                                               \
	    rectangle rect,                                                 \
	    void* val                                                       \
	) {                                                                 \
	    return dataGui(ctx, layId, rect, (dataRef**)val, &data##Table); \
	}

// specializations of dataGui
SELECTOR_GUI(texture)
SELECTOR_GUI(mesh)
SELECTOR_GUI(material)
SELECTOR_GUI(script)

// selects shader data
int shaderGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// selects a path 
int pathGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// -- callbacks

// makes data selector callback
renderCallback makeDataselCallback(
	dataRef** ref,
	dataTable* tab,
	guiContext* orig
);

// makes shader selector callback
renderCallback makeShaderselCallback(dataRef** ref, guiContext* orig);

// makes path selector callback
renderCallback makePathselCallback(
	char* path,
	guiContext* orig,
	char* curPath
);

#endif
