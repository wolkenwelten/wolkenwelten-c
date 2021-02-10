#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chunk.h"

#define CHUNGUS_SIZE (16*CHUNK_SIZE)

struct chungus {
	u8 x,y,z;
	u64 requested;
	void *nextFree;
	beingList bl;
	chunk *chunks[16][16][16];
};

typedef struct {
	float distance;
	union {
		chunk   *chnk;
		chungus *chng;
	};
} queueEntry;

void     chungusInit           ();
chungus *chungusNew            (u8 x,u8 y, u8 z);
void     chungusFree           (chungus *c);
void     chungusBox            (chungus *c, u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, u8 block);
void     chungusBoxF           (chungus *c, u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, u8 block);
void     chungusSetB           (chungus *c, u16 x, u16 y, u16 z, u8 block);
u8       chungusGetB           (chungus *c, u16 x, u16 y, u16 z);
ivec     chungusGetPos         (const chungus *c);
chunk   *chungusGetChunk       (chungus *c, u16 x, u16 y, u16 z);
chunk   *chungusGetChunkOrNew  (chungus *c, u16 x, u16 y, u16 z);
void     chungusQueueDraws     (chungus *c,const character *cam, queueEntry *drawQueue,int *drawQueueLen);
chungus *chungusGetActive      (uint i);
void     chungusSetActiveCount (uint i);
uint     chungusGetActiveCount ();
