#ifndef EDITOR_H
#define EDITOR_H

// -- icons

// row 4
#define ICO_ENTITY GUI_ICO(0,  4)
#define ICO_INT    GUI_ICO(1,  4)
#define ICO_FLOAT  GUI_ICO(2,  4)
#define ICO_STRING GUI_ICO(3,  4)
#define ICO_SCENE  GUI_ICO(4,  4)
#define ICO_FLOAT2 GUI_ICO(5,  4)
#define ICO_FLOAT3 GUI_ICO(6,  4)
#define ICO_FLOAT4 GUI_ICO(7,  4)
#define ICO_MAT2   GUI_ICO(8,  4)
#define ICO_MAT3   GUI_ICO(9,  4)
#define ICO_MAT4   GUI_ICO(10, 4)
#define ICO_QUAT   GUI_ICO(11, 4)
#define ICO_TRANS  GUI_ICO(12, 4)

// row 5
#define ICO_NEW    GUI_ICO(0,  5)
#define ICO_DELETE GUI_ICO(1,  5)
#define ICO_MOVE   GUI_ICO(2,  5)
#define ICO_INDENT GUI_ICO(3,  5)
#define ICO_TEX    GUI_ICO(4,  5)
#define ICO_MESH   GUI_ICO(5,  5)
#define ICO_MAT    GUI_ICO(6,  5)
#define ICO_FILE   GUI_ICO(8,  5)
#define ICO_NUFILE GUI_ICO(9,  5)
#define ICO_CAMERA GUI_ICO(10, 5)
#define ICO_ATMOS  GUI_ICO(11, 5)
#define ICO_DIR    GUI_ICO(12, 5)
#define ICO_EMPTY  GUI_ICO(13, 5)
#define ICO_LOAD   GUI_ICO(14, 5)
#define ICO_SAVE   GUI_ICO(15, 5)

// row 6
#define ICO_TABLE  GUI_ICO(0,  6)
#define ICO_SCRIPT GUI_ICO(1,  6)
#define ICO_SHADER GUI_ICO(2,  6)

// -- constants

// default data directory
#define DATA_DIR "dat"

// -- windows

// forward declaration for createEditorWindows 
typedef struct scene scene;

// creates editor windows
void createEditorWindows(scene* scn);

#endif
