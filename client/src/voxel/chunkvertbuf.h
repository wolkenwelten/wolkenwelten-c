#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chunk.h"

void chunkvertbufInit        ();
u32  chunkvertbufUsedBytes   ();
u32  chunkvertbufMaxBytes    ();
void chunkvertbufUpdate      (chunk *c, vertexPacked *vertices, u16 sideVtxCounts[sideMAX]);
void chunkvertbufFree        (chunk *c);
void chunkvertbufDrawQueue   (queueEntry *queue, int queueLen);
void chunkvertbufDrawOne     (chunk *c, sideMask mask);
