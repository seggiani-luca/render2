#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// -- initialization

// OpenGL versioning
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 3

// update OpenGL events
void updateGl();

// terminate OpenGL
void freeGl();

// -- icons

// windowIcon definition
typedef GLFWimage windowIcon;

// icon paths
#define WIN_DEFAULT_ICO "dat/icon/default.tga"
#define WIN_HIERCHY_ICO "dat/icon/hierarchy.tga"
#define WIN_INSPECT_ICO "dat/icon/inspector.tga"
#define WIN_NEWITEM_ICO "dat/icon/new.tga"
#define WIN_DATASEL_ICO "dat/icon/datasel.tga"

// loads icons
windowIcon* loadIcon(const char* path);

// -- windows

// forward declaration for renderCallback
typedef struct window window;

// callback for window rendering
typedef struct {
	// actual rendering function
	void (*fun)(window* win);

	// context for callback
	void* ctx;

	// context freeing function
	void (*free)(void* ctx);
} renderCallback;

// window data
struct window {
	// window dimensions
	int width, height;
	
	// framebuffer dimensions
	int fbWidth, fbHeight;

	// title of window
	const char* title;

	// rendering callback for window
	renderCallback cbak;

	// GLFW window handle
	GLFWwindow* gl;
};

// creates a new window
window* newWindow(
	int width,
	int height,
	const char* title,
	renderCallback cback,
	windowIcon* ico,
	int depth
);

// updates a window
int updateWindow(window* win);

// frees a window
void freeWindow(window* win);

// resizes a window
void resizeWindow(window* win, int width, int height);

// moves a window
void moveWindow(window* win, int x, int y);

// width from window coordinate to framebuffer coordinate
float winToFbW(window* win, float from);

// height from window coordinate to framebuffer coordinate
float winToFbH(window* win, float from);

// width from framebuffer coordinate to window coordinate
float fbToWinW(window* win, float from);

// height from framebuffer coordinate to window coordinate
float fbToWinH(window* win, float from);

#endif
