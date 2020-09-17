#pragma once
#include <stdint.h>
#include <time.h>

typedef struct {
	unsigned int ID,w,h;
	char *filename;
	time_t modTime;
} texture;

#pragma pack(push, 1)
typedef struct vertex {
	float x,y,z;
	float u,v;
	float c;
} vertex;

typedef struct vertex2D {
	 int16_t x,y;
	 int16_t u,v;
	uint32_t rgba;
} vertex2D;
#pragma pack(pop)

typedef struct {
	vertex2D dataBuffer[1<<14];
	int sx,sy,size;
	int dataCount;
	int vboSize;
	unsigned int vbo,usage;
	texture *tex;
	int finished;
	void *nextFree;
} textMesh;

typedef struct {
	texture *tex;
	const vertex *roData;
	unsigned int dataCount;
	unsigned int vbo;
	void *nextFree;
} mesh;

typedef struct {
	unsigned int pID;
	unsigned int vsID,fsID;
	char *vss,*fss;
	unsigned int attrMask;
	int lMVP,lAlpha,lTransform,lTex;
} shader;

typedef struct {
	union {
		struct {
			float x,y,z;
		};
		struct {
			float u,v,w;
		};
		struct {
			float yaw,pitch,roll;
		};
	};
} vec;
