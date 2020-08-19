#pragma once
#include "../../../common/src/common.h"

textMesh *textMeshNew       ();
void      textMeshFree      (textMesh *m);
void      textMeshEmpty     (textMesh *m);
void      textMeshAddVert   (textMesh *m, int16_t x, int16_t y, int16_t u, int16_t v, uint32_t rgba);
void      textMeshDraw      (textMesh *m);
void      textMeshAddGlyph  (textMesh *m, int x, int y, int size, unsigned char c);
void      textMeshAddStrPS  (textMesh *m, int x, int y, int size, const char *str);
void      textMeshAddString (textMesh *m, const char *str);
void      textMeshPrintfPS  (textMesh *m, int x, int y, int size, const char *format, ...);
void      textMeshPrintf    (textMesh *m, const char *format, ...);
void      textMeshDigit     (textMesh *m, int x, int y, int size, int digit);
void      textMeshNumber    (textMesh *m, int x, int y, int size, int number);
void      textMeshBox       (textMesh *m, int x, int y, int w, int h, float u, float v, float uw, float vh, uint32_t rgba);
void      textMeshSolidBox  (textMesh *m, int x, int y, int w, int h, uint32_t rgba);
void      textMeshItemSprite(textMesh *m, int x, int y, int size, int itemID);
void      textMeshSlot      (textMesh *m, int x, int y, int size, int style);
void      textMeshItemSlot  (textMesh *m, int x, int y, int size, int style, int itemID, int amount);
void      textMeshItem      (textMesh *m, int x, int y, int size, int style, item *itm);
