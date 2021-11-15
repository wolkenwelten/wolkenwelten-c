#pragma once
#include "../../../common/src/common.h"

#define ITEMTILE (1.f/32.f)

extern u32 colorPalette[16];

textMesh *textMeshNew              (uint bufferSize);
void      textMeshFree             (textMesh *m);
void      textMeshEmpty            (textMesh *m);
void      textMeshAddVert          (textMesh *m, i16 x, i16 y, i16 u, i16 v, u32 rgba);
void      textMeshDraw             (textMesh *m);
void      textMeshAddGlyph         (textMesh *m, int x, int y, int size, u8 c, u32 fgc, u32 bgc);
void      textMeshAddGlyphHG       (textMesh *m, int x, int y, int size, u8 c, u32 fgc, u32 bgc1, u32 bgc2);
int       textMeshAddWrappedPS     (textMesh *m, int x, int y, int w, int h, int size, const char *str);
bool      textMeshAddStrPS         (textMesh *m, int x, int y, int size, const char *str);
bool      textMeshAddString        (textMesh *m, const char *str);
bool      textMeshPrintfPS         (textMesh *m, int x, int y, int size, const char *format, ...);
bool      textMeshPrintf           (textMesh *m, const char *format, ...);
bool      textMeshPrintfAlignCenter(textMesh *m, const char *format, ...);
bool      textMeshPrintfAlignRight (textMesh *m, const char *format, ...);
void      textMeshDigit            (textMesh *m, int x, int y, int size, int digit);
void      textMeshNumber           (textMesh *m, int x, int y, int size, int number);
void      textMeshBox              (textMesh *m, int x, int y, int w, int h, float u, float v, float uw, float vh, u32 rgba);
void      textMeshSolidBox         (textMesh *m, int x, int y, int w, int h, u32 rgba);
void      textMeshHGradient        (textMesh *m, int x, int y, int w, int h, u32 c1, u32 c2);
void      textMeshVGradient        (textMesh *m, int x, int y, int w, int h, u32 c1, u32 c2);
void      textMeshItemSprite       (textMesh *m, int x, int y, int size, int itemID);
void      textMeshSlot             (textMesh *m, int x, int y, int size, int style);
void      textMeshItemSlot         (textMesh *m, int x, int y, int size, int style, int itemID, int amount);
void      textMeshItem             (textMesh *m, int x, int y, int size, int style, const item *itm);
void      textMeshResetFont        (textMesh *m);
