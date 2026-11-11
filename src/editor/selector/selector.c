#include "selector.h"
#include "../editor.h"
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
	char path[GUI_IN_BUF_SIZ];

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
	char vert[GUI_IN_BUF_SIZ];
	
	// path of frag 
	char frag[GUI_IN_BUF_SIZ];

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
	char curPath[GUI_IN_BUF_SIZ];	

	// original GUI context
	guiContext* orig;
} pathselGuiContext;

// renders the data selector GUI
void dataselGui(window* win) {
	// get context
	dataselGuiContext* dCtx = (dataselGuiContext*)guiInit(win);
	if(!dCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &dCtx->gui;
	dataTable* tab = dCtx->tab;
	char* path = dCtx->path;
	guiContext* orig = dCtx->orig;

	// update input state
	guiInput(win);

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0,
		WIN_WIDTH, WIN_HEIGHT 
	}, GUI_BG_ABS);

	// push bottom frame
	guiPushQuad(ctx, FIXED, (guiRect) {
		0, WIN_HEIGHT - GUI_TXT_HEIGHT - 4 GUI_PAD,
		WIN_WIDTH, GUI_TXT_HEIGHT + 4 GUI_PAD
	}, GUI_BG_LIGHT);

	// push label
	guiText(ctx, FIXED, (guiPos){
		1 GUI_PAD, WIN_HEIGHT - GUI_TXT_HEIGHT - 2 GUI_PAD
	}, "Path:");

	// push path edit box
	pathGui(ctx, FIXED, (guiRect){
		1 GUI_PAD + PATH_OFF, WIN_HEIGHT - GUI_TXT_HEIGHT - 3 GUI_PAD,
		IMPORT_OFF - 2 GUI_PAD - PATH_OFF, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, path);

	// push import button
	if(guiButton(ctx, FIXED, (guiRect){
		IMPORT_OFF, WIN_HEIGHT - GUI_TXT_HEIGHT - 3 GUI_PAD,
		WIN_WIDTH - IMPORT_OFF - 1 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
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
	guiScroll(ctx, SCROLL);

	// go through references, pushing to gui
	dataRef* cur = tab->root;
	while(cur) {
		char str[DAT_PATH_SIZ + 16];
		snprintf(str, DAT_PATH_SIZ + 16, "%s (%d refs)", 
				cur->path, cur->refCount);

		if(guiButton(ctx, SCROLL, (guiRect){
			1 GUI_PAD, 1 GUI_PAD,
			WIN_WIDTH - 2 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
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

		guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);
		cur = cur->next;
	}

	guiDown(ctx, SCROLL, 1 GUI_PAD);

	// flush changes
	guiFlush(ctx);
}

// renders the shader selector GUI
void shaderselGui(window* win) {
	// get context
	shaderselGuiContext* dCtx = (shaderselGuiContext*)guiInit(win);
	if(!dCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &dCtx->gui;
	char* vert = dCtx->vert;
	char* frag = dCtx->frag;
	guiContext* orig = dCtx->orig;

	// update input state
	guiInput(win);

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0,
		WIN_WIDTH, WIN_HEIGHT
	}, GUI_BG_ABS);

	// push bottom frame
	guiPushQuad(ctx, FIXED, (guiRect) {
		0, WIN_HEIGHT - GUI_TXT_HEIGHT - 4 GUI_PAD,
		WIN_WIDTH, GUI_TXT_HEIGHT + 4 GUI_PAD
	}, GUI_BG_LIGHT);

	// push vert label
	guiText(ctx, FIXED, (guiPos){
		1 GUI_PAD, WIN_HEIGHT - GUI_TXT_HEIGHT - 2 GUI_PAD
	}, "Vert:");

	// push vert edit box
	pathGui(ctx, FIXED, (guiRect){
		1 GUI_PAD + PATH_OFF, WIN_HEIGHT - GUI_TXT_HEIGHT - 3 GUI_PAD,
		IMPORT_OFF / 2 - 2 GUI_PAD - PATH_OFF, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, vert);
	
	// push frag label
	guiText(ctx, FIXED, (guiPos){
		1 GUI_PAD + IMPORT_OFF / 2, WIN_HEIGHT - GUI_TXT_HEIGHT - 2 GUI_PAD
	}, "Frag:");

	// push frag edit box
	pathGui(ctx, FIXED, (guiRect){
		1 GUI_PAD + PATH_OFF + IMPORT_OFF / 2, WIN_HEIGHT - GUI_TXT_HEIGHT - 3 GUI_PAD,
		IMPORT_OFF / 2 - 2 GUI_PAD - PATH_OFF, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, frag);

	// push import button
	if(guiButton(ctx, FIXED, (guiRect){
		IMPORT_OFF, WIN_HEIGHT - GUI_TXT_HEIGHT - 3 GUI_PAD,
		WIN_WIDTH - IMPORT_OFF - 1 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
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
	guiScroll(ctx, SCROLL);

	// go through references, pushing to gui
	dataRef* cur = shaderTable.root;
	while(cur) {
		char str[DAT_PATH_SIZ + 16];
		snprintf(str, DAT_PATH_SIZ + 16, "%s (%d refs)", 
				cur->path, cur->refCount);

		if(guiButton(ctx, SCROLL, (guiRect){
			1 GUI_PAD, 1 GUI_PAD,
			WIN_WIDTH - 2 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
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

		guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);
		cur = cur->next;
	}

	guiDown(ctx, SCROLL, 1 GUI_PAD);

	// flush changes
	guiFlush(ctx);
}

// renders the path selector GUI
void pathselGui(window* win) {
	// get context
	pathselGuiContext* pCtx = (pathselGuiContext*)guiInit(win);
	if(!pCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &pCtx->gui;
	char* path = pCtx->path;
	guiContext* orig = pCtx->orig;
	char* curPath = pCtx->curPath;

	// update input state
	guiInput(win);

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0,
		WIN_WIDTH, WIN_HEIGHT 
	}, GUI_BG_ABS);

	// open data directory if needed
	if(!pCtx->cur) pCtx->cur = opendir(curPath);
	DIR* cur = pCtx->cur;
	
	// scroll explorer layer
	guiScroll(ctx, SCROLL);

	// push upwards button
	if(guiButton(ctx, SCROLL, (guiRect){
		1 GUI_PAD, 1 GUI_PAD,
		WIN_WIDTH - 2 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
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
	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);

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

		if(guiButton(ctx, SCROLL, (guiRect){
			1 GUI_PAD, 1 GUI_PAD,
			WIN_WIDTH - 2 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
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

		guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);
	}

	// rewind directory
	rewinddir(cur);

	guiDown(ctx, SCROLL, 1 GUI_PAD);

	// flush changes
	guiFlush(ctx);
}

// -- callbacks

winRenderCallback makeDataselCallback(
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
	return (winRenderCallback){
		dataselGui,
		dCtx,
		guiFree	
	};
}

winRenderCallback makeShaderselCallback(dataRef** ref, guiContext* orig) {
	// initialize context
	shaderselGuiContext* dCtx = xmalloc(sizeof(shaderselGuiContext));
	dCtx->gui.win = NULL;
	dCtx->gui.child = NULL;
	dCtx->ref = ref;
	*dCtx->vert = '\0';
	*dCtx->frag = '\0';
	dCtx->orig = orig;

	// return callback
	return (winRenderCallback){
		shaderselGui,
		dCtx,
		guiFree	
	};
}

// frees path selector context
void freePathsel(void* vCtx) {
	pathselGuiContext* ctx = (pathselGuiContext*)vCtx;
	
	// close open directory
	if(ctx->cur) closedir(ctx->cur);
	
	guiFree(vCtx);
}

winRenderCallback makePathselCallback(
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
	return (winRenderCallback){
		pathselGui,
		pCtx,
		freePathsel	
	};
}

// -- selectors

int dataGui(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	dataRef** ref,
	dataTable* tab
) {
	// ref to string 
	char str[GUI_OUT_BUF_SIZ];
	snprintf(str, GUI_OUT_BUF_SIZ, "%.*s", GUI_OUT_BUF_SIZ - 1, 
		*ref ? (*ref)->path : "(null path)");

	// make gui
	rect.w -= 3 GUI_PAD + GUI_ICO_SIZ;
	guiTextBox(ctx, layId, rect, str);

	// search button
	if(guiButton(ctx, SCROLL, (guiRect){
		rect.x + rect.w + 1 GUI_PAD, rect.y,
		2 GUI_PAD + GUI_ICO_SIZ, 1 GUI_PAD + GUI_ICO_SIZ 
	}, ICO_TABLE, "")) {
		// set data pointer in context
		ctx->in.dataPtr = ref;

		// make datasel
		guiChildWindow(ctx, winNew(
			DATASEL_WIDTH,
			DATASEL_HEIGHT,
			"Select Data",
			makeDataselCallback(ref, tab, ctx),
			NULL,
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

int shaderGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val) {
	dataRef** ref = val;

	// ref to string 
	char str[GUI_OUT_BUF_SIZ];
	snprintf(str, GUI_OUT_BUF_SIZ, "%.*s", GUI_OUT_BUF_SIZ - 1, 
		*ref ? (*ref)->path : "(null path)");

	// make gui
	rect.w -= 3 GUI_PAD + GUI_ICO_SIZ;
	guiTextBox(ctx, layId, rect, str);

	// search button
	if(guiButton(ctx, SCROLL, (guiRect){
		rect.x + rect.w + 1 GUI_PAD, rect.y,
		2 GUI_PAD + GUI_ICO_SIZ, 1 GUI_PAD + GUI_ICO_SIZ 
	}, ICO_TABLE, "")) {
		// set data pointer in context
		ctx->in.dataPtr = ref;

		// make datasel
		guiChildWindow(ctx, winNew(
			DATASEL_WIDTH,
			DATASEL_HEIGHT,
			"Select Data",
			makeShaderselCallback(ref, ctx),
			NULL,
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

int pathGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val) {
	// push non-interactive box
	guiTextBox(ctx, layId, rect, (char*)val);

	if(guiPress(ctx, layId, rect)) {
		// set data pointer in context
		ctx->in.dataPtr = val;

		guiChildWindow(ctx, winNew(
			PATHSEL_WIDTH,
			PATHSEL_HEIGHT,
			"Select File",
			makePathselCallback(val, ctx, DATA_DIR),
			NULL,
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
