#include "hierarchy.h"
#include "../editor.h"
#include "../selector/selector.h"
#include "../inspector/inspector.h"
#include "../../exception/exception.h"
#include <stdlib.h>
#include <string.h>

// -- contexts

// context for scene GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// scene
	scene* scn;

	// entity to move
	entity* moving;

	// path of scene to load
	char load[GUI_IN_BUF_SIZ];
} sceneGuiContext;

// context for new child GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// entity
	entity* ent;
	
	// scene
	scene* scn;

	// name buffer
	char name[GUI_IN_BUF_SIZ];

	// path of entity to load 
	char load[GUI_IN_BUF_SIZ];
} addChildGuiContext;

// hook into inspector window
extern window* inspectorWin;

// enum for hierarchy action
typedef enum {
	NONE = 0,
	DELETE = 1,
	MOVE = 2,
	INSPECT = 3,
	NEW = 4
} hierAction;

// -- scenes

// renders the add field GUI
void addChildGui(window* win) {
	// get context
	addChildGuiContext* aCtx = (addChildGuiContext*)guiInit(win);
	if(!aCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &aCtx->gui;
	entity* ent = aCtx->ent;
	scene* scn = aCtx->scn;
	char* name = aCtx->name;

	// update input state
	guiInput(win);

	// realize early if entity path changed
	if(ctx->in.dataPtr == aCtx->load && ctx->in.dataSet) {
		changeEntityCallback(inspectorWin, NULL);

		// load entity 
		entity* new = deserializeEntityFile(aCtx->load);
		if(new) appendChild(ent, new);
		ctx->in.dataSet = 0;
		
		// flag dirty
		scn->dirty = 1;

		// should close
		glfwSetWindowShouldClose(win->gl, 1);
	}

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0,
		WIN_WIDTH, WIN_HEIGHT 
	}, GUI_BG_ABS);

	// push name edit box
	{
		// mask
		guiPushQuad(ctx, SCROLL, (guiRect){
			0, 0,
			WIN_WIDTH, GUI_TXT_HEIGHT + 4 GUI_PAD
		}, GUI_BG_ABS);

		// push label
		guiText(ctx, SCROLL, (guiPos){
			1 GUI_PAD, 2 GUI_PAD
		}, "Name:");

		// push name edit box
		stringGui(ctx, SCROLL, (guiRect){
			1 GUI_PAD + CHILD_OFF, 1 GUI_PAD,
			WIN_WIDTH - 5 GUI_PAD - GUI_ICO_SIZ - CHILD_OFF, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, name);

		if(guiButton(ctx, SCROLL, (guiRect){
			WIN_WIDTH - 7 GUI_PAD, 1 GUI_PAD,
			2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ICO_LOAD, "")) {	
			// prepare to load entity 
			ctx->in.dataPtr = aCtx->load;
			guiChildWindow(ctx, winNew(
				PATHSEL_WIDTH,
				PATHSEL_HEIGHT,
				"Select Entity",
				makePathselCallback(aCtx->load, ctx, ENTITY_DIR),
				NULL,
				WIN_DEPTH_OFF
			));
		}
	}
	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);

	// declare child button table
	typedef struct {
		const char* label;
		guiRect ico;
		entity* (*ctor)(const char* name);
	} childButton;
	static const childButton childButtons[] = {
		{ "New Child",  ICO_ENTITY, newRenderableEntity },
		{ "New Sun",    ICO_ATMOS,  newSunEntity        },
		{ "New Camera", ICO_CAMERA, newCameraEntity     },
		{ "New Empty",  ICO_EMPTY,  newEntity           }
	};
	int childButtonCount = (int)(sizeof(childButtons) / sizeof(childButton));

	for(int i = 0; i < childButtonCount; i++) {
		const childButton* c = &childButtons[i];

		// push new child button
		if(guiButton(ctx, SCROLL, (guiRect){
			1 GUI_PAD, 1 GUI_PAD,
			WIN_WIDTH - 2 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, c->ico, c->label)) {
			// no objects named the same as root
			if(strcmp(name, ROOT_NAME) == 0) break; 

			// create and append child
			entity* new = c->ctor(name);
			if(new) appendChild(ent, new);

			// flag dirty
			scn->dirty = 1;

			// should close
			glfwSetWindowShouldClose(win->gl, 1);
		}
		guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);
	}

	guiDown(ctx, SCROLL, 1 GUI_PAD);
	guiTrim(ctx, SCROLL);

	// flush changes
	guiFlush(ctx);
}

// gets a callback object for a "new field" menu
winRenderCallback makeAddChildCallback(entity* ent, scene* scn) {
	// initialize context
	addChildGuiContext* aCtx = xmalloc(sizeof(addChildGuiContext));
	aCtx->gui.win = NULL;
	aCtx->gui.child = NULL;
	aCtx->ent = ent;
	aCtx->scn = scn;
	*aCtx->name = '\0';

	// return callback
	return (winRenderCallback){
		addChildGui,
		aCtx,
		guiFree
	};
}

// renders an entity in the scene hierarchy to GUI
hierAction entityHierGui(guiContext* ctx, entity* ent, int depth) {
	hierAction act = NONE;

	// push icon
	if(depth > 0) {
		guiIcon(ctx, SCROLL, (guiPos){
			1 GUI_PAD + (depth - 1) * (GUI_ICO_SIZ + 1 GUI_PAD), 1 GUI_PAD
		}, ICO_INDENT);
	}

	// push entity button
	if(guiButton(ctx, SCROLL, (guiRect){
		1 GUI_PAD + depth * (GUI_ICO_SIZ + 1 GUI_PAD), 1 GUI_PAD,
		HIER_ELEM_WIDTH, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, ICO_ENTITY, ent->name)) {
		act = INSPECT;
	}

	// push new child button
	if(guiButton(ctx, SCROLL, (guiRect){
		WIN_WIDTH - 3 GUI_PAD - GUI_ICO_SIZ, 1 GUI_PAD,
		2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, ICO_NEW, "")) {
		act = NEW;
	}

	if(depth != 0) {
		// push delete child button
		if(guiButton(ctx, SCROLL, (guiRect){
			WIN_WIDTH - 6 GUI_PAD - 2 * GUI_ICO_SIZ, 1 GUI_PAD,
			2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ICO_DELETE, "")) {
			act = DELETE;
		}

		// push move child button
		if(guiButton(ctx, SCROLL, (guiRect){
			WIN_WIDTH - 9 GUI_PAD - 3 * GUI_ICO_SIZ, 1 GUI_PAD,
			2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ICO_MOVE, "")) {
			act = MOVE;
		}
	}

	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);

	return act;
}

// renders the hierarchy GUI
void sceneGui(window* win) {
	// get context
	sceneGuiContext* sCtx = (sceneGuiContext*)guiInit(win);
	if(!sCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &sCtx->gui;
	scene* scn = sCtx->scn;

	// update input state
	guiInput(win);

	// realize early if scene path changed
	if(ctx->in.dataPtr == sCtx->load && ctx->in.dataSet) {
		changeEntityCallback(inspectorWin, NULL);

		// load scene
		deserializeSceneFile(scn, sCtx->load);
		ctx->in.dataSet = 0;
	}

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0,
		WIN_WIDTH, WIN_HEIGHT 
	}, GUI_BG_ABS);

	// push scene label
	{
		// mask
		guiPushQuad(ctx, FIXED, (guiRect){
			0, 0,
			WIN_WIDTH, GUI_TXT_HEIGHT + 4 GUI_PAD
		}, GUI_BG_ABS);

		// push icon
		guiIcon(ctx, FIXED, (guiPos){
			2 GUI_PAD, 2 GUI_PAD
		}, ICO_SCENE);

		// push name edit box
		stringGui(ctx, FIXED, (guiRect){
			3 GUI_PAD + GUI_ICO_SIZ, 1 GUI_PAD,
			WIN_WIDTH - 10 GUI_PAD - 3 * GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, scn ? scn->name : NULL);

		// push load button
		if(guiButton(ctx, FIXED, (guiRect){
			WIN_WIDTH - 10 GUI_PAD - GUI_ICO_SIZ, 1 GUI_PAD,
			2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ICO_LOAD, "")) {	
			// prepare to load scene 
			ctx->in.dataPtr = sCtx->load;
			guiChildWindow(ctx, winNew(
				PATHSEL_WIDTH,
				PATHSEL_HEIGHT,
				"Select Scene",
				makePathselCallback(sCtx->load, ctx, SCENE_DIR),
				NULL,
				WIN_DEPTH_OFF
			));
		}
		
		// push save button
		if(guiButton(ctx, FIXED, (guiRect){
			WIN_WIDTH - 3 GUI_PAD - GUI_ICO_SIZ, 1 GUI_PAD,
			2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ICO_SAVE, "")) {	
			if(*scn->name == '\0') {
				logEvent(ERROR, IO, "Couldn't serialize scene with empty path");
				return;
			}

			// save scene
			serializeSceneFile(scn, getScenePath(scn->name));

			return; // early quit
		}
	}
	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);

	// scroll field layer
	guiScroll(ctx, SCROLL);

	// go through scene hierarchy
	sceneIter iter = getScIter(scn);
	for(;;) {
		entity* cur = iter.cur;
		int depth = iter.depth;

		// advance first
		entity* next = scIterNext(&iter);

		// get action
		hierAction act = entityHierGui(ctx, cur, depth);
		switch(act) {
			case DELETE: {
				// reset move
				sCtx->moving = NULL;

				// make sure to update inspector
				if(getEntityCallback(inspectorWin) == cur)
					changeEntityCallback(inspectorWin, NULL);

				// remove entity from parent 
				removeChild(cur->parent, cur);
				
				// free entity
				freeEntity(cur);

				// flag dirty
				scn->dirty = 1;
				
				// tree is not really valid anymore, redraw at next iteration
				return;
			} break;

			case MOVE: {
				// start move
				sCtx->moving = cur;			
			} break;

			case INSPECT: {
				entity* moving = sCtx->moving;

				if(moving) {
					// finish move
					moveChild(cur, moving);

					// reset move
					sCtx->moving = NULL;
				
					// flag dirty
					scn->dirty = 1;
				}
				else {
					// view
					if(cur != &scn->root)
						changeEntityCallback(inspectorWin, cur);
				}
			} break;

			case NEW: {
				// reset move
				sCtx->moving = NULL;

				// create new child window
				guiChildWindow(ctx, winNew(
					ADD_CHILD_WIDTH,
					ADD_CHILD_HEIGHT,
					"Append Child",
					makeAddChildCallback(cur, scn),
					NULL,
					WIN_DEPTH_OFF
				));
			} break;

			default: break;
		}

		if(!next) break;
	}
	guiDown(ctx, SCROLL, 1 GUI_PAD);

	// flush changes
	guiFlush(ctx);
}

winRenderCallback makeSceneCallback(scene* scn) {
	// initialize context
	sceneGuiContext* sCtx = xmalloc(sizeof(sceneGuiContext));
	sCtx->gui.win = NULL;
	sCtx->gui.child = NULL;
	sCtx->scn = scn;
	sCtx->moving = NULL;

	// return callback
	return (winRenderCallback){
		sceneGui,
		sCtx,
		guiFree
	};
}


