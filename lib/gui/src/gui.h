/**
 * Minimal GUI library based on window, a library for OpenGL windows.
 * Designed to be extensible, example widget definitions are given in the
 * widget/ directory.
 *
 * 2026 - Luca Seggiani
 */

#ifndef GUI_H
#define GUI_H

/*
 * GUI theme to use. Available themes:
 * - GUI_THEME_DARK
 * - GUI_THEME_LIGHT
 */
#define GUI_THEME_DARK

/**
 * Basic GUI rendering, state and input handling.
 */
#include "backend/backend.h"

/**
 * GUI drawing primitives, basic widgets and complex widget examples.
 */
#include "widget/widget.h"

#endif
