#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chunk.h"

#pragma pack(push, 1)
typedef struct vertexTiny {
	u8 x,y,z,u,v,w,f;
} vertexTiny;
#pragma pack(pop)

struct chunkvertbuf;

void chunkvertbufInit        ();
u32  chunkvertbufUsedBytes   ();
u32  chunkvertbufMaxBytes    ();
void chunkvertbufUpdate      (chunk *c, vertexTiny *vertices, u16 sideCounts[sideMAX]);
void chunkvertbufFree        (chunk *c);
void chunkvertbufDrawQueue   (queueEntry *queue, int queueLen);
void chunkvertbufDrawOne     (chunk *c, sideMask mask);
