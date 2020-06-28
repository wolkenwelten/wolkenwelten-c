#pragma once
#include "../gfx/texture.h"
#include <stdint.h>

#pragma pack(push, 1)
typedef struct vertex {
	float x,y,z;
	float u,v;
	float c;
} vertex;
#pragma pack(pop)

typedef struct {
	texture *tex;
	vertex *roData;
	unsigned int dataCount;
	unsigned int vbo;
	void *nextFree;
} mesh;

mesh *meshNew      ();
mesh *meshNewRO    (vertex *nroData,size_t roSize);
void  meshFreeAll  ();
void  meshFree     (mesh *m);
void  meshAddVert  (mesh *m, float x,float y,float z,float u,float v);
void  meshAddVertC (mesh *m, float x,float y,float z,float u,float v,float c);
void  meshFinish   (mesh *m, unsigned int usage);
void  meshDraw     (mesh *m);
void  meshDrawLin  (mesh *m);
void  meshDrawVBO  (mesh *m);
