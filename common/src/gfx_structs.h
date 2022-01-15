#pragma once
#include "stdint.h"
#include <time.h>
#include "../nujel/lib/api.h"

typedef struct {
	uint ID,w,h,d;
	const char *filename;
} texture;

#pragma pack(push, 1)
typedef struct vertex {
	float x,y,z;
	float u,v;
	float c;
} vertex;

typedef struct vertexFlat {
	float x,y,z;
	u32 rgba;
} vertexFlat;

typedef struct vertex2D {
	i16 x,y;
	i16 u,v;
	u32 rgba;
} vertex2D;
#pragma pack(pop)

#define VERTEX_PACKED_X_LEN 5
#define VERTEX_PACKED_X_OFFSET 0
#define VERTEX_PACKED_Y_LEN 5
#define VERTEX_PACKED_Y_OFFSET 5
#define VERTEX_PACKED_Z_LEN 5
#define VERTEX_PACKED_Z_OFFSET 10
#define VERTEX_PACKED_BT_LEN 8
#define VERTEX_PACKED_BT_OFFSET 16
#define VERTEX_PACKED_SIDE_LEN 3
#define VERTEX_PACKED_SIDE_OFFSET 24
typedef uint32_t vertexPacked;
#define mkVertex(x,y,z,w,h,bt,side) (\
	((x) << VERTEX_PACKED_X_OFFSET) |\
	((y) << VERTEX_PACKED_Y_OFFSET) |\
	((z) << VERTEX_PACKED_Z_OFFSET) |\
	((side) << VERTEX_PACKED_SIDE_OFFSET) |\
	((bt) << VERTEX_PACKED_BT_OFFSET)\
)

typedef struct queueEntry queueEntry;
typedef struct chunkvertbuf chunkvertbuf;

typedef struct {
	vertex2D *dataBuffer;
	int sx,sy,mx,my,wrap,size,font;
	u32 fgc, bgc;
	uint vao,vbo,usage,dataCount,bufferSize,vboSize;
	texture *tex;
	int finished;
} textMesh;

typedef struct {
	texture *tex;
	const vertex *roData;
	uint dataCount,vboSize;
	uint vbo,vao;
	void *nextFree;
} mesh;

typedef struct {
	uint pID;
	uint vsID,fsID;
	const char *vss,*fss;
	const char *defines;
	const char *name;
	uint attrMask;
	int lMVP,lAlpha,lColor,lTransform,lSideTints,lSizeMul;
} shader;

typedef struct {
	uint ID;
	uint width;
	uint height;
	uint texColor, rboDepth;
} framebufferObject;

#pragma pack(push, 1)
typedef struct {
	float x,y,z,size;
} rainDrop;

typedef struct {
	float x,y,z,size;
} snowDrop;
#pragma pack(pop)
