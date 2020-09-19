#pragma once
#include "../../../common/src/common.h"
#include "../game/character.h"
#include "../voxel/chunk.h"

#include <stddef.h>

#define CHUNGUS_SIZE (16*16)

struct chungus {
	int x,y,z,loaded;
	void *nextFree;
	chunk *chunks[16][16][16];
};

typedef struct {
	float distance;
	union {
		chunk   *chnk;
		chungus *chng;
	};
} queueEntry;

chungus *chungusNew            (int x,int y, int z);
void     chungusFree           (chungus *c);
void     chungusBox            (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void     chungusBoxF           (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void     chungusSetB           (chungus *c, int x, int y, int z, u8 block);
u8       chungusGetB           (chungus *c, int x, int y, int z);
chunk   *chungusGetChunk       (chungus *c, int x, int y, int z);
chunk   *chungusGetChunkOrNew  (chungus *c, int x, int y, int z);
void     chungusQueueDraws     (chungus *c,const character *cam, queueEntry *drawQueue,int *drawQueueLen);
chungus *chungusGetActive      (uint i);
void     chungusSetActiveCount (uint i);
uint     chungusGetActiveCount ();
