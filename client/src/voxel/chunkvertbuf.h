#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chunk.h"

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
#define mkVertexPacked(x,y,z,w,h,bt,side) (\
	((x) << VERTEX_PACKED_X_OFFSET) |\
	((y) << VERTEX_PACKED_Y_OFFSET) |\
	((z) << VERTEX_PACKED_Z_OFFSET) |\
	((side) << VERTEX_PACKED_SIDE_OFFSET) |\
	((bt) << VERTEX_PACKED_BT_OFFSET)\
)

struct chunkvertbuf;

void chunkvertbufInit        ();
u32  chunkvertbufUsedBytes   ();
u32  chunkvertbufMaxBytes    ();
void chunkvertbufUpdate      (chunk *c, vertexPacked *vertices, u16 sideVtxCounts[sideMAX]);
void chunkvertbufFree        (chunk *c);
void chunkvertbufDrawQueue   (queueEntry *queue, int queueLen);
void chunkvertbufDrawOne     (chunk *c, sideMask mask);
