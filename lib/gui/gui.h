/*
 * Minimal GUI library based on window, a library for OpenGL windows.
 * 
 * Designed to be extensible, example widget definitions are given in the
 * src/widget/ directory.
 * Themes are available in the src/gui.h header.
 *
 * 2026 - Luca Seggiani
 */

#ifndef LIB_GUI_H
#define LIB_GUI_H

#include "src/gui.h"

// -- errors

/**
 * Reports an error that happened in the library, for instance from a returned
 * NULL.
 *
 * @return a string representing the last error that happened
 */
const char* guiError();

#endif
