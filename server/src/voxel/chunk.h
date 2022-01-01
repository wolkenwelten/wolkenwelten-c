#pragma once
#include "../../../common/src/common.h"

extern const float CHUNK_RENDER_DISTANCE;
extern const float CHUNK_FADEOUT_DISTANCE;

struct chunk {
	u16   x,y,z;
	u32   clientsUpdated;
	void *nextFree;
	beingList bl;

	chunkOverlay *fluid, *block;
};

extern uint chunkCount;
extern chunk *chunkList;

void   chunkInit        ();
chunk *chunkNew         (u16 x,u16 y,u16 z);
void   chunkFree        (chunk *c);
void   chunkFill        (chunk *c, blockId b);
void   chunkGetB        (chunk *c, int x, int y, int z);
void   chunkSetB        (chunk *c, int x, int y, int z, blockId b);
void   chunkBox         (chunk *c, int x,int y,int z,int gx,int gy,int gz,blockId block);
int    chunkIsUpdated   (chunk *c, uint p);
void   chunkSetUpdated  (chunk *c, uint p);
void   chunkUnsetUpdated(chunk *c, uint p);
uint   chunkGetFree     ();
uint   chunkGetActive   ();
float  chunkDistance    (const vec pos, const chunk *chnk);
void   chunkCheckShed   ();
