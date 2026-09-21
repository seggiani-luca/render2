/**
 * Minimal window library for simple OpenGL windows using GLFW.
 * Example usage:
 *
 * winRenderCallback cback; // hooked to content rendering functions
 * winIcon* icon;           // loaded as a pixel map
 *
 * window* win = winNew(
 *     150, 100,
 *     "Title",
 *     cback, icon,
 *     DEPTH_OFF
 * )
 *
 * while(winUpdate(win));
 *
 * freeWin(win);
 *
 * 2026 - Luca Seggiani
 */

#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdint.h>

// -- initialization

/**
 * Major OpenGL version
 */
#define WIN_GL_VERSION_MAJOR 3

/**
 * Minor OpenGL version
 */
#define WIN_GL_VERSION_MINOR 3

/**
 * Signals if the library should try to use Wayland. Otherwise, X11 will be
 * used. Defaults to x11, as Wayland doesn't allow some features such as window
 * moving.
 */
#define WIN_USE_WAYLAND 0 

/**
 * Depth test on.
 */
#define DEPTH_ON 1

/**
 * Depth test off.
 */
#define DEPTH_OFF 0

// -- icons

/**
 * Definition of a window icon, boxes GLFWimage.
 */
typedef GLFWimage winIcon;

/**
 * Loads a window icon.
 *
 * @param  height height of icon
 * @param  width  width of icon
 * @param  data   pixel data in RGBA format, top left origin
 * @return        the resulting window icon if valid, NULL otherwise
 */
winIcon* winLoadIcon(int height, int width, uint8_t* data);

// -- windows

/**
 * Window rendering callback. Called to query window contents on window
 * updates.
 * Rendering callbacks are expected to live on the stack: if state is needed,
 * a context pointer is offered.
 */
typedef struct window window;
typedef struct {
	/**
	 * The function that actually renders window contents.
	 *
	 * @param win the window to update
	 */
	void (*fun)(window* win);

	/**
	 * Context of the content rendering function. Can store any state the
	 * function needs to keep between frames to render.
	 */
	void* ctx;

	/**
	 * An (optional) function that gets called to destroy the context of the
	 * content rendering function.
	 */
	void (*free)(void* ctx);
} winRenderCallback;

/**
 * Window definition.
 */
struct window {
	/**
	 * Window dimensions, width and height. Corresponds to logical screen
	 * dimensions, used by the window system for coordinates, etc...
	 */
	int width, height;

	/**
	 * Window framebuffer dimensions, width and height. Corresponds to the
	 * actual size of the OpenGL framebuffer window contents are rendered to.
	 */
	int fbWidth, fbHeight;

	/**
	 * Title of the window.
	 */
	const char* title;

	/**
	 * The window's rendering callback, used to query window contents on window
	 * updates.
	 */
	winRenderCallback cbak;

	/**
	 * Handle for the underlying GLFW window.
	 */
	GLFWwindow* gl;
};

/**
 * Creates a new window. Expects a fully formed render callback and window
 * icon: in case of failure, makes sure to free the given resources.
 * The first window to be created is taken as the "root window", and its 
 * context is shared across all subsequent windows. Use this window as the main
 * window of the program.
 *
 * @param  width  logical width of window
 * @param  height logical height of window
 * @param  title  title of window
 * @param  cback  rendering callback of window
 * @param  ico    icon of window
 * @param depth   whether to enable depth testing on window or not
 * @return        the resulting window if valid, NULL otherwise
 */
window* winNew(
	int width,
	int height,
	const char* title,
	winRenderCallback cback,
	winIcon* ico,
	int depth
);

/**
 * Updates a window.
 *
 * @param  win window to update
 * @return     a return code signaling window status:
 *             -  1: window is alive
 *             -  0: window asked to be closed, caller should winFree() it 
 */
int winUpdate(window* win);

/**
 * Closes a window, freeing resources. When the last window is freed, library
 * state is automatically reset. The root window shouldn't be freed before
 * other windows: doing so will inevitably leak memory shared between windows. 
 *
 * @param win window to free 
 */
void winFree(window* win);

/**
 * Resizes a window. 
 *
 * @param win    window to resize 
 * @param width  new logical width of window
 * @param height new logical height of window
 */
void winResize(window* win, int width, int height);

/**
 * Moves a window. 
 *
 * @param win window to move 
 * @param x   new logical x coordinate of window 
 * @param y   new logical y coordinate of window
 */
void winMove(window* win, int x, int y);

/**
 * Signals a window to close.
 */
void winClose(window* win);

// -- conversions

/**
 * Converts width from logical coordinates to framebuffer coordinates.
 *
 * @param win  window this transformation relates to
 * @param from width in logical coordinates
 * @return     width in framebuffer coordinates
 */
float winToFbW(window* win, float from);

/**
 * Converts height from logical coordinates to framebuffer coordinates.
 *
 * @param win  window this transformation relates to
 * @param from height in logical coordinates
 * @return     height in framebfufer coordinates
 */
float winToFbH(window* win, float from);

/**
 * Converts width from framebuffer coordinates to logical coordinates.
 *
 * @param win  window this transformation relates to
 * @param from width in framebuffer coordinates
 * @return     width in logical coordinates
 */
float fbToWinW(window* win, float from);

/**
 * Converts height from framebuffer coordinates to logical coordinates.
 *
 * @param win  window this transformation relates to
 * @param from height in framebuffer coordinates
 * @return     height in logical coordinates
 */
float fbToWinH(window* win, float from);

// -- errors

/**
 * Reports an error that happened in the library, for instance from a returned
 * NULL.
 *
 * @return a string representing the last error that happened
 */
const char* winError();

#endif
