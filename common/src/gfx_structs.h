#pragma once
#include "stdint.h"
#include <time.h>

typedef struct {
	uint ID,w,h;
	const char *filename;
	time_t modTime;
} texture;

#pragma pack(push, 1)
typedef struct vertex {
	float x,y,z;
	float u,v;
	float c;
} vertex;

typedef struct vertex2D {
	i16 x,y;
	i16 u,v;
	u32 rgba;
} vertex2D;
#pragma pack(pop)

typedef struct {
	vertex2D *dataBuffer;
	 int sx,sy,mx,my,wrap,size,font;
	 u32 fgc, bgc;
	uint vbo,usage,dataCount,bufferSize;
	texture *tex;
	 int finished;
} textMesh;

typedef struct {
	texture *tex;
	const vertex *roData;
	uint dataCount;
	uint vbo;
	void *nextFree;
} mesh;

typedef struct {
	uint pID;
	uint vsID,fsID;
	char *vss,*fss;
	uint attrMask;
	 int lMVP,lAlpha,lTransform,lTex;
} shader;

typedef struct {
	union {
		struct { float x,y,z; };
		struct { float u,v,w; };
		struct { float yaw,pitch,roll; };
	};
} vec;

typedef struct {
	union {
		struct { int x,y,z; };
		struct { int w,h,d; };
	};
} ivec;

typedef struct {
	union {
		struct { uint x,y,z; };
		struct { uint w,h,d; };
	};
} uvec;
