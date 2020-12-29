#pragma once
#include "../../../common/src/common.h"

#define CHUNK_SIZE (16)
extern const float CHUNK_RENDER_DISTANCE;
extern const float CHUNK_FADEOUT_DISTANCE;

struct chunk{
	u16   x,y,z;
	u64   clientsUpdated;
	void *nextFree;
	beingList bl;
	u8    data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

extern uint chunkCount;
extern chunk *chunkList;

void   chunkInit        ();
chunk *chunkNew         (u16 x,u16 y,u16 z);
void   chunkFree        (chunk *c);
void   chunkFill        (chunk *c, u8 b);
void   chunkGetB        (chunk *c, int x, int y, int z);
void   chunkSetB        (chunk *c, int x, int y, int z, u8 b);
int    chunkIsUpdated   (chunk *c, uint p);
void   chunkSetUpdated  (chunk *c, uint p);
void   chunkUnsetUpdated(chunk *c, uint p);
uint   chunkGetFree     ();
uint   chunkGetActive   ();
float  chunkDistance    (const vec pos, const chunk *chnk);
void   chunkCheckShed   ();

static inline void chunkBox(chunk *c, int x,int y,int z,int gx,int gy,int gz,u8 block){
	for(int cx = x; cx < gx; cx++){
	for(int cy = y; cy < gy; cy++){
	for(int cz = z; cz < gz; cz++){
		c->data[cx][cy][cz] = block;
	}
	}
	}
	c->clientsUpdated = 0;
}
