#ifndef EDITOR_H
#define EDITOR_H

#include "../gui/gui.h"

// -- icons

// row 4
#define ICO_ENTITY ICO(0,  4)
#define ICO_INT    ICO(1,  4)
#define ICO_FLOAT  ICO(2,  4)
#define ICO_STRING ICO(3,  4)
#define ICO_SCENE  ICO(4,  4)
#define ICO_FLOAT2 ICO(5,  4)
#define ICO_FLOAT3 ICO(6,  4)
#define ICO_FLOAT4 ICO(7,  4)
#define ICO_MAT2   ICO(8,  4)
#define ICO_MAT3   ICO(9,  4)
#define ICO_MAT4   ICO(10, 4)
#define ICO_QUAT   ICO(11, 4)
#define ICO_TRANS  ICO(12, 4)

// row 5
#define ICO_NEW    ICO(0,  5)
#define ICO_DELETE ICO(1,  5)
#define ICO_MOVE   ICO(2,  5)
#define ICO_INDENT ICO(3,  5)
#define ICO_TEX    ICO(4,  5)
#define ICO_MESH   ICO(5,  5)
#define ICO_MAT    ICO(6,  5)
#define ICO_FILE   ICO(8,  5)
#define ICO_NUFILE ICO(9,  5)
#define ICO_CAMERA ICO(10, 5)
#define ICO_ATMOS  ICO(11, 5)
#define ICO_DIR    ICO(12, 5)
#define ICO_EMPTY  ICO(13, 5)
#define ICO_LOAD   ICO(14, 5)
#define ICO_SAVE   ICO(15, 5)

// row 6
#define ICO_TABLE  ICO(0,  6)
#define ICO_SCRIPT ICO(1,  6)
#define ICO_SHADER ICO(2,  6)

// -- constants

// default data directory
#define DATA_DIR "dat"

// -- windows

// forward declaration for createEditorWindows 
typedef struct scene scene;

// creates editor windows
void createEditorWindows(scene* scn);

#endif
