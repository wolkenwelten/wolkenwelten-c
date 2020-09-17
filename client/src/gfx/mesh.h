#pragma once
#include "../../../common/src/common.h"

mesh *meshNew      ();
mesh *meshNewRO    (const vertex *nroData,size_t roSize);
void  meshFreeAll  ();
void  meshFree     (mesh *m);
void  meshAddVert  (mesh *m, float x,float y,float z,float u,float v);
void  meshAddVertC (mesh *m, float x,float y,float z,float u,float v,float c);
void  meshFinish   (mesh *m, unsigned int usage);
void  meshDraw     (const mesh *m);
void  meshDrawLin  (const mesh *m);
void  meshDrawVBO  (const mesh *m);
