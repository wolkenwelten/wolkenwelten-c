#pragma once
#include "../../../common/src/common.h"

mesh *meshNew           ();
mesh *meshNewRO         (const vertex *nroData,size_t roSize);
mesh *meshGet           (uint i);
void  meshFreeAll       ();
void  meshFree          (mesh *m);
void  meshEmpty         (mesh *m);
void  meshAddVert       (mesh *m, float x,float y,float z,float u,float v);
void  meshAddVertC      (mesh *m, float x,float y,float z,float u,float v,float c);
void  meshFinishStatic  (mesh *m);
void  meshFinishDynamic (mesh *m);
void  meshDraw          (const mesh *m);
