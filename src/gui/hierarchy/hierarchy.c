#include "hierarchy.h"
#include "../inspector/inspector.h"
#include "../widget/widget.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

// context for scene GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// scene
	scene* scn;
} sceneGuiContext;

// context for new child GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// entity
	entity* ent;

	// name buffer
	char name[ENT_NAME_SIZ];
} addChildGuiContext;

// hook into inspector window
extern window* inspectorWin;

// -- scenes

// renders the add field GUI 
void addChildGui(window* win) {
	// get context
	addChildGuiContext* eCtx = (addChildGuiContext*) initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;
	char* name = eCtx->name;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4) {
		0,
		0,
		WIN,
		ADD_FIELD_HEIGHT	
	}, BG_ABS);

	// push name edit box
	{
		// mask
		quadGui(ctx, FIXED, (float4) {
			0,
			0,
			WIN,
			TXT_HEIGHT + 4 PAD 
		}, BG_ABS);

		// push label
		textGui(ctx, FIXED, (float2){
			1 PAD,
			2 PAD
		}, "Name:");

		// push name edit box
		stringGui(ctx, FIXED, (float4) {
			1 PAD + NEW_OFF,
			1 PAD,
			WIN - 2 PAD - NEW_OFF, 
			TXT_HEIGHT + 2 PAD
		}, name);
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// push new field button
	if(buttonGui(ctx, SCROLL, (float4) {
		1 PAD, 
		1 PAD,
		WIN        - 2 PAD, 
		TXT_HEIGHT + 2 PAD
	}, ICO_ENTITY, "New Child")) {
		// actually create and append child
		appendChild(ent, newEntity(name));

		// should close
		glfwSetWindowShouldClose(win->gl, 1);
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// flush changes
	flushGui(ctx);
}

// gets a callback object for a "new field" menu 
renderCallback makeAddChildCallback(entity* ent) {
	// initialize context
	addChildGuiContext* eCtx = malloc(sizeof(addChildGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;
	*eCtx->name = '\0';

	// return callback
	return (renderCallback) {
		addChildGui,
		eCtx,
		freeGui
	};
}
		
// renders an entity in the scene hierarchy to GUI
void entityHierGui(guiContext* ctx, entity* ent, int depth) {
	// push icon
	if(depth > 0) {
		iconGui(ctx, SCROLL, (float2) {
			1 PAD + (depth - 1) * (ICO_SIZ + 1 PAD),
			1 PAD
		}, ICO_INDENT);
	}	

	// push entity button
	if(buttonGui(ctx, SCROLL, (float4) {
		1 PAD + depth * (ICO_SIZ + 1 PAD),
		1 PAD,
		HIER_ELEM_WIDTH,
		TXT_HEIGHT + 2 PAD
	}, ICO_ENTITY, ent->name)) {
		changeEntityCallback(inspectorWin, ent);
	}

	// push new child button
	if(buttonGui(ctx, SCROLL, (float4) {
		2 PAD + depth * (ICO_SIZ + 1 PAD) + HIER_ELEM_WIDTH,
		1 PAD,
		CHILD_WIDTH,
		TXT_HEIGHT + 2 PAD
	}, ICO_NEW, "Append")) {
		// create new child window
		subWindowGui(ctx, newWindow(
			ADD_CHILD_WIDTH,
			ADD_CHILD_HEIGHT,
			"Append Child",
			makeAddChildCallback(ent)
		));
	}

	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
}

// renders the hierarchy GUI 
void sceneGui(window* win) {
	// get context
	sceneGuiContext* sCtx = (sceneGuiContext*) initGui(win);
	guiContext* ctx = &sCtx->gui;
	scene* scn = sCtx->scn;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4) {
		0,
		0,
		WIN,
		HIERARCHY_HEIGHT	
	}, BG_ABS);

	// push scene label
	{
		// mask
		quadGui(ctx, FIXED, (float4) {
			0,
			0,
			WIN,
			TXT_HEIGHT + 4 PAD 
		}, BG_ABS);

		// push icon
		iconGui(ctx, FIXED, (float2) {
			2 PAD, 
			2 PAD
		}, ICO_SCENE);

		// push name edit box
		stringGui(ctx, FIXED, (float4) {
			3 PAD + ICO_SIZ, 
			1 PAD, 
			WIN        - 4 PAD - ICO_SIZ, 
			TXT_HEIGHT + 2 PAD
		}, scn ? scn->name : NULL);
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// go through scene hierarchy
	sceneIter iter = getScIter(scn);
	do {
		// push entity
		entityHierGui(ctx, iter.cur, iter.depth);
	} while(scIterNext(&iter));

	// flush changes
	flushGui(ctx);
}

renderCallback makeSceneCallback(scene* scn) {
	// initialize context
	sceneGuiContext* sCtx = malloc(sizeof(sceneGuiContext));
	sCtx->gui.win = NULL;
	sCtx->gui.child = NULL;
	sCtx->scn = scn;

	// return callback
	return (renderCallback) {
		sceneGui,
		sCtx,
		freeGui
	};
}
