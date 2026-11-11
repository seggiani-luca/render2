#ifndef GUI_H
#define GUI_H

/*
 * GUI theme to use. Some default themes are made available:
 * - GUI_THEME_DARK: default dark theme
 * - GUI_THEME_LIGHT: default light theme
 * - GUI_THEME_NEO: phosphor green terminal, matrix-like 
 * - GUI_THEME_TUX: icy, arctic whites and blues 
 * - GUI_THEME_TOKYO: port of tokyo night theme 
 */
#define GUI_THEME_TOKYO

/**
 * Basic GUI rendering, state and input handling.
 */
#include "backend/backend.h"

/**
 * GUI drawing primitives, basic widgets and complex widget examples.
 */
#include "widget/widget.h"

#endif
