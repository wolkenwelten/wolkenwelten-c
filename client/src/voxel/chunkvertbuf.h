#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chunk.h"

void chunkvertbufInit        ();
u32  chunkvertbufUsedBytes   ();
u32  chunkvertbufMaxBytes    ();
void chunkvertbufFree        (chunk *c);
void chunkvertbufDrawQueue   (queueEntry *queue, int queueLen);
void chunkvertbufDrawOne     (sideMask mask, chunkvertbuf *v);
chunkvertbuf *chunkvertbufBlockUpdate(chunkvertbuf *v, vertexPacked *vertices, u16 sideVtxCounts[sideMAX]);
chunkvertbuf *chunkvertbufFluidUpdate(chunkvertbuf *v, vertexFluid *vertices, u16 sideVtxCounts[sideMAX]);
