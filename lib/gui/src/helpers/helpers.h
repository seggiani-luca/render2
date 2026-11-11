#ifndef GUI_HELPERS_H
#define GUI_HELPERS_H

#include "../../../glad/glad.h"

/**
 * Compiles shader for the GUI.
 *
 * @param vert path of GUI vert shader
 * @param frag path of GUI frag shader
 * @return GLuint handle of GUI shader
 */
GLuint guiImportShader(const char* vertPath, const char* fragPath);

/**
 * Frees shader for the GUI.
 *
 * @param progam GLuint of GUI shader to free
 */
void guiFreeShader(GLuint program);

/**
 * Load texture for the GUI.
 *
 * @param path of GUI theme
 * @return GLuint handle of GUI texture
 */
GLuint guiImportTexture(const char* path);

/**
 * Frees texture for the GUI.
 *
 * @param progam GLuint of GUI texture to free
 */
void guiFreeTexture(GLuint texture);

#endif
