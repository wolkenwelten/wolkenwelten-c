#pragma once
#include "../../../../common/src/common.h"

#define CUBE_FACES 6
#define VERTICES_PER_FACE 4

extern int chunksGeneratedThisFrame;
uint chunkGetGeneratedThisFrame();

sideMask chunkGetSides              (u16 x,u16 y,u16 z,blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2]);
void     chunkPopulateBlockData     (blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff);
void     chunkOptimizePlane         (u64 plane[CHUNK_SIZE][CHUNK_SIZE]);
uint     chunkGetGeneratedThisFrame ();
void     chunkResetCounter          ();
