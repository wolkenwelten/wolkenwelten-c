#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chunk.h"

void       blockMeshInit        ();
u32        blockMeshUsedBytes   ();
u32        blockMeshMaxBytes    ();
void       blockMeshFree        (chunk *c);
void       blockMeshDrawQueue   (queueEntry *queue, int queueLen);
void       blockMeshDrawOne     (sideMask mask, blockMesh *v);
blockMesh *blockMeshUpdate      (blockMesh *v, vertexPacked *vertices, u16 sideVtxCounts[sideMAX]);
blockMesh *blockMeshFluidUpdate (blockMesh *v, vertexFluid *vertices, u16 sideVtxCounts[sideMAX]);
