#include "selector.h"
#include "../editor.h"
#include "../../gui/widget/widget.h"
#include <dirent.h>
#include <string.h>

// -- contexts

// context for data selector GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// data reference to update 
	dataRef** ref;

	// data table to update from 
	dataTable* tab;

	// path of data to find
	char path[IN_BUF_SIZ];

	// original GUI context
	guiContext* orig;
} dataselGuiContext;

// context for shader selector GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// data reference to update 
	dataRef** ref;

	// path of vert 
	char vert[IN_BUF_SIZ];
	
	// path of frag 
	char frag[IN_BUF_SIZ];

	// original GUI context
	guiContext* orig;
} shaderselGuiContext;

// context for path selector GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// path to update 
	char* path;
	
	// current directory 
	DIR* cur;

	// current directory patih
	char curPath[IN_BUF_SIZ];	

	// original GUI context
	guiContext* orig;
} pathselGuiContext;

// renders the data selector GUI
void dataselGui(window* win) {
	// get context
	dataselGuiContext* dCtx = (dataselGuiContext*)initGui(win);
	guiContext* ctx = &dCtx->gui;
	dataTable* tab = dCtx->tab;
	char* path = dCtx->path;
	guiContext* orig = dCtx->orig;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (rectangle){
		0, 0,
		WIN, HEIG 
	}, BG_ABS);

	// push bottom frame
	quadGui(ctx, FIXED, (rectangle) {
		0, HEIG - TXT_HEIGHT - 4 PAD,
		WIN, TXT_HEIGHT + 4 PAD
	}, BG_LIGHT);

	// push label
	textGui(ctx, FIXED, (position){
		1 PAD, HEIG - TXT_HEIGHT - 2 PAD
	}, "Path:");

	// push path edit box
	pathGui(ctx, FIXED, (rectangle){
		1 PAD + PATH_OFF, HEIG - TXT_HEIGHT - 3 PAD,
		IMPORT_OFF - 2 PAD - PATH_OFF, TXT_HEIGHT + 2 PAD
	}, path);

	// push import button
	if(buttonGui(ctx, FIXED, (rectangle){
		IMPORT_OFF, HEIG - TXT_HEIGHT - 3 PAD,
		WIN - IMPORT_OFF - 1 PAD, TXT_HEIGHT + 2 PAD
	}, ICO_NUFILE, "Import")) {
		// import data
		if((*dCtx->ref)) freeData((*dCtx->ref)->data, tab);
		*dCtx->ref = importData(path, tab);
		
		// update original context
		orig->in.dataSet = 1;

		// should close
		glfwSetWindowShouldClose(win->gl, 1);
	}
	
	// scroll reference layer
	scrollGui(ctx, SCROLL);

	// go through references, pushing to gui
	dataRef* cur = tab->root;
	while(cur) {
		char str[DAT_PATH_SIZ + 16];
		snprintf(str, DAT_PATH_SIZ + 16, "%s (%d refs)", 
				cur->path, cur->refCount);

		if(buttonGui(ctx, SCROLL, (rectangle){
			1 PAD, 1 PAD,
			WIN - 2 PAD, TXT_HEIGHT + 2 PAD
		}, ICO_FILE, str)) {
			// keep path, unspeakable things may happen
			char path[DAT_PATH_SIZ];
			strcpy(path, cur->path);

			// use this reference
			if((*dCtx->ref)) freeData((*dCtx->ref)->data, tab);
			*dCtx->ref = importData(path, tab);
		
			// update original context
			orig->in.dataSet = 1;

			// should close
			glfwSetWindowShouldClose(win->gl, 1);

			// early return
			return;
		}

		downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
		cur = cur->next;
	}

	downGui(ctx, SCROLL, 1 PAD);

	// flush changes
	flushGui(ctx);
}

// renders the shader selector GUI
void shaderselGui(window* win) {
	// get context
	shaderselGuiContext* dCtx = (shaderselGuiContext*)initGui(win);
	guiContext* ctx = &dCtx->gui;
	char* vert = dCtx->vert;
	char* frag = dCtx->frag;
	guiContext* orig = dCtx->orig;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (rectangle){
		0, 0,
		WIN, HEIG 
	}, BG_ABS);

	// push bottom frame
	quadGui(ctx, FIXED, (rectangle) {
		0, HEIG - TXT_HEIGHT - 4 PAD,
		WIN, TXT_HEIGHT + 4 PAD
	}, BG_LIGHT);

	// push vert label
	textGui(ctx, FIXED, (position){
		1 PAD, HEIG - TXT_HEIGHT - 2 PAD
	}, "Vert:");

	// push vert edit box
	pathGui(ctx, FIXED, (rectangle){
		1 PAD + PATH_OFF, HEIG - TXT_HEIGHT - 3 PAD,
		IMPORT_OFF / 2 - 2 PAD - PATH_OFF, TXT_HEIGHT + 2 PAD
	}, vert);
	
	// push frag label
	textGui(ctx, FIXED, (position){
		1 PAD + IMPORT_OFF / 2, HEIG - TXT_HEIGHT - 2 PAD
	}, "Frag:");

	// push frag edit box
	pathGui(ctx, FIXED, (rectangle){
		1 PAD + PATH_OFF + IMPORT_OFF / 2, HEIG - TXT_HEIGHT - 3 PAD,
		IMPORT_OFF / 2 - 2 PAD - PATH_OFF, TXT_HEIGHT + 2 PAD
	}, frag);

	// push import button
	if(buttonGui(ctx, FIXED, (rectangle){
		IMPORT_OFF, HEIG - TXT_HEIGHT - 3 PAD,
		WIN - IMPORT_OFF - 1 PAD, TXT_HEIGHT + 2 PAD
	}, ICO_NUFILE, "Import")) {
		// import data
		if((*dCtx->ref)) shaderFree((*dCtx->ref)->data);
		*dCtx->ref = shaderImport(vert, frag);
		
		// update original context
		orig->in.dataSet = 1;

		// should close
		glfwSetWindowShouldClose(win->gl, 1);
	}
	
	// scroll reference layer
	scrollGui(ctx, SCROLL);

	// go through references, pushing to gui
	dataRef* cur = shaderTable.root;
	while(cur) {
		char str[DAT_PATH_SIZ + 16];
		snprintf(str, DAT_PATH_SIZ + 16, "%s (%d refs)", 
				cur->path, cur->refCount);

		if(buttonGui(ctx, SCROLL, (rectangle){
			1 PAD, 1 PAD,
			WIN - 2 PAD, TXT_HEIGHT + 2 PAD
		}, ICO_FILE, str)) {
			// keep path, unspeakable things may happen
			char path[DAT_PATH_SIZ];
			strcpy(path, cur->path);

			// split path
			char* vert;
			char* frag;
			splitShaderPath(path, &vert, &frag);

			// use this reference
			if((*dCtx->ref)) shaderFree((*dCtx->ref)->data);
			*dCtx->ref = shaderImport(vert, frag);
		
			// update original context
			orig->in.dataSet = 1;

			// should close
			glfwSetWindowShouldClose(win->gl, 1);

			// early return
			return;
		}

		downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
		cur = cur->next;
	}

	downGui(ctx, SCROLL, 1 PAD);

	// flush changes
	flushGui(ctx);
}

// renders the path selector GUI
void pathselGui(window* win) {
	// get context
	pathselGuiContext* pCtx = (pathselGuiContext*)initGui(win);
	guiContext* ctx = &pCtx->gui;
	char* path = pCtx->path;
	guiContext* orig = pCtx->orig;
	char* curPath = pCtx->curPath;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (rectangle){
		0, 0,
		WIN, HEIG 
	}, BG_ABS);

	// open data directory if needed
	if(!pCtx->cur) pCtx->cur = opendir(curPath);
	DIR* cur = pCtx->cur;
	
	// scroll explorer layer
	scrollGui(ctx, SCROLL);

	// push upwards button
	if(buttonGui(ctx, SCROLL, (rectangle){
		1 PAD, 1 PAD,
		WIN - 2 PAD, TXT_HEIGHT + 2 PAD
	}, ICO_MOVE, "..")) {
		// remove last directory from path
		char *slash = strrchr(pCtx->curPath, '/');

		// don't go above the filesystem root
		if(slash && slash != pCtx->curPath) *slash = '\0';

		// open new directory
		DIR *next = opendir(pCtx->curPath);
		if(!next) return;

		// close old directory
		closedir(pCtx->cur);
		
		pCtx->cur = next;

		return; // early exit;
	}	
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// go through entries, pushing to gui
	struct dirent *ent;
	while ((ent = readdir(cur)) != NULL) {
		// skip . and ..
		if(strcmp(ent->d_name, "." ) == 0
		|| strcmp(ent->d_name, "..") == 0) continue;

		// file or directory?
		if(ent->d_type != DT_REG
		&& ent->d_type != DT_DIR) continue;
		int dir = ent->d_type == DT_DIR;

		// name of entry
		char str[DAT_PATH_SIZ + 16];
		snprintf(
			str,
			DAT_PATH_SIZ,
			"%s",
			ent->d_name
		);

		if(buttonGui(ctx, SCROLL, (rectangle){
			1 PAD, 1 PAD,
			WIN - 2 PAD, TXT_HEIGHT + 2 PAD
		}, dir ? ICO_DIR : ICO_FILE, str)) {
			if(dir) {
				// build new directory path
				char newPath[DAT_PATH_SIZ + sizeof(ent->d_name) + 1];
				snprintf(
					newPath,
					sizeof(newPath),
					"%s/%s",
					curPath,
					ent->d_name
				);
				if(strlen(newPath) >= DAT_PATH_SIZ - 1) return;

				// open new directory
				DIR *next = opendir(newPath);
				if(!next) return;

				// close old directory
				closedir(pCtx->cur);
				
				pCtx->cur = next;
				strncpy(pCtx->curPath, newPath, DAT_PATH_SIZ);
				pCtx->curPath[DAT_PATH_SIZ - 1] = '\0';

				return; // early exit;
			} else {
				// build final path
				char newPath[DAT_PATH_SIZ + sizeof(ent->d_name) + 1];
				snprintf(
					newPath,
					sizeof(newPath),
					"%s/%s",
					curPath,
					ent->d_name
				);
				if(strlen(newPath) >= DAT_PATH_SIZ - 1) return;

				// select this file
				orig->in.dataSet = 1;
				strncpy(path, newPath, DAT_PATH_SIZ);
				path[DAT_PATH_SIZ - 1] = '\0';
				
				glfwSetWindowShouldClose(ctx->win->gl, 1);
			}
		}

		downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
	}

	// rewind directory
	rewinddir(cur);

	downGui(ctx, SCROLL, 1 PAD);

	// flush changes
	flushGui(ctx);
}

// -- callbacks

renderCallback makeDataselCallback(
	dataRef** ref,
	dataTable* tab,
	guiContext* orig
) {
	// initialize context
	dataselGuiContext* dCtx = xmalloc(sizeof(dataselGuiContext));
	dCtx->gui.win = NULL;
	dCtx->gui.child = NULL;
	dCtx->ref = ref;
	dCtx->tab = tab;
	*dCtx->path = '\0';
	dCtx->orig = orig;

	// return callback
	return (renderCallback){
		dataselGui,
		dCtx,
		freeGui
	};
}

renderCallback makeShaderselCallback(dataRef** ref, guiContext* orig) {
	// initialize context
	shaderselGuiContext* dCtx = xmalloc(sizeof(shaderselGuiContext));
	dCtx->gui.win = NULL;
	dCtx->gui.child = NULL;
	dCtx->ref = ref;
	*dCtx->vert = '\0';
	*dCtx->frag = '\0';
	dCtx->orig = orig;

	// return callback
	return (renderCallback){
		shaderselGui,
		dCtx,
		freeGui
	};
}

// frees path selector context
void freePathsel(void* vCtx) {
	pathselGuiContext* ctx = (pathselGuiContext*)vCtx;
	
	// close open directory
	if(ctx->cur) closedir(ctx->cur);
	
	freeGui(vCtx);
}

renderCallback makePathselCallback(
	char* path,
	guiContext* orig,
	char* curPath
) {
	// initialize context
	pathselGuiContext* pCtx = xmalloc(sizeof(pathselGuiContext));
	pCtx->gui.win = NULL;
	pCtx->gui.child = NULL;
	pCtx->path = path;
	pCtx->cur = NULL;
	pCtx->orig = orig;
	
	// init path
	strncpy(pCtx->curPath, curPath, DAT_PATH_SIZ);
	pCtx->curPath[DAT_PATH_SIZ - 1] = '\0';

	// return callback
	return (renderCallback){
		pathselGui,
		pCtx,
		freePathsel	
	};
}

// -- selectors

int dataGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	dataRef** ref,
	dataTable* tab
) {
	// ref to string 
	char str[OUT_BUF_SIZ];
	snprintf(str, OUT_BUF_SIZ, "%.*s", OUT_BUF_SIZ - 1, 
		*ref ? (*ref)->path : "(null path)");

	// make gui
	rect.w -= 3 PAD + ICO_SIZ;
	textBoxGui(ctx, layId, rect, str);

	// search button
	if(buttonGui(ctx, SCROLL, (rectangle){
		rect.x + rect.w + 1 PAD, rect.y,
		2 PAD + ICO_SIZ, 1 PAD + ICO_SIZ 
	}, ICO_TABLE, "")) {
		// set data pointer in context
		ctx->in.dataPtr = ref;

		// make datasel
		subWindowGui(ctx, newWindow(
			DATASEL_WIDTH,
			DATASEL_HEIGHT,
			"Select Data",
			makeDataselCallback(ref, tab, ctx),
			loadIcon(WIN_DATASEL_ICO),
			0
		));
	}

	// return if changed
	if(ctx->in.dataPtr == ref && ctx->in.dataSet) {
		ctx->in.dataSet = 0;
		return 1;
	}

	return 0;
}

int shaderGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) {
	dataRef** ref = val;

	// ref to string 
	char str[OUT_BUF_SIZ];
	snprintf(str, OUT_BUF_SIZ, "%.*s", OUT_BUF_SIZ - 1, 
		*ref ? (*ref)->path : "(null path)");

	// make gui
	rect.w -= 3 PAD + ICO_SIZ;
	textBoxGui(ctx, layId, rect, str);

	// search button
	if(buttonGui(ctx, SCROLL, (rectangle){
		rect.x + rect.w + 1 PAD, rect.y,
		2 PAD + ICO_SIZ, 1 PAD + ICO_SIZ 
	}, ICO_TABLE, "")) {
		// set data pointer in context
		ctx->in.dataPtr = ref;

		// make datasel
		subWindowGui(ctx, newWindow(
			DATASEL_WIDTH,
			DATASEL_HEIGHT,
			"Select Data",
			makeShaderselCallback(ref, ctx),
			loadIcon(WIN_DATASEL_ICO),
			0
		));
	}

	// return if changed
	if(ctx->in.dataPtr == ref && ctx->in.dataSet) {
		ctx->in.dataSet = 0;
		return 1;
	}

	return 0;
}

int pathGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) {
	// push non-interactive box
	textBoxGui(ctx, layId, rect, (char*)val);

	if(pressGui(ctx, layId, rect)) {
		// set data pointer in context
		ctx->in.dataPtr = val;

		subWindowGui(ctx, newWindow(
			PATHSEL_WIDTH,
			PATHSEL_HEIGHT,
			"Select File",
			makePathselCallback(val, ctx, DATA_DIR),
			loadIcon(WIN_PATHSEL_ICO),
			0
		));
	}
		
	// return if found
	if(ctx->in.dataPtr == val && ctx->in.dataSet) {
		ctx->in.dataSet = 0;
		return 1;
	}

	return 0;
}
