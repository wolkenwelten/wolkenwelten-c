#pragma once
#include "../../../../common/src/common.h"

#define FADE_IN_FRAMES 48

void chunkPopulateBlockData(blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff);
void chunkGenBlockMesh(chunk *c);
