#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"
#include "chunkvertbuf.h"

extern u64 chunksDirtied;
extern u64 chunksCopied;

#define CHUNK_FLAG_DIRTY 1
#define CHUNK_FLAG_FLUID_DIRTY 2
#define CHUNK_MASK_DIRTY (CHUNK_FLAG_DIRTY | CHUNK_FLAG_FLUID_DIRTY)

struct chunk {
	u16 x,y,z;
	u8 flags;
	u8 fadeIn;
	u16 framesSkipped;

	chunkvertbuf *blockVertbuf, *fluidVertbuf;
	beingList bl;

	chunkOverlay *fluid, *block, *fire, *light;
};

void    chunkInit                  ();
void    chunkReset                 (chunk *c, int x, int y, int z);
void    chunkFree                  (chunk *c);
void    chunkDrawBlockQueue        (const queueEntry *queue, int queueLen);
void    chunkDrawFluidQueue        (const queueEntry *queue, int queueLen);
void    chunkBox                   (chunk *c, u16 x, u16 y, u16 z,u16 gx,u16 gy,u16 gz,blockId b);
void    chunkSetB                  (chunk *c, u16 x, u16 y, u16 z, blockId b);
void    chunkRecvUpdate            (const packet *p);
void    chunkRecvEmpty             (const packet *p);
bool    chunkInFrustum             (const chunk *c);
void    chunkTryDirty              (int x, int y, int z);
void    chunkDirtyRegion           (int cx, int cy, int cz, uint flag);
void    chunkDirtyAll              ();
void    chunkGenBlockMesh          (chunk *c);
void    chunkGenFluidMesh          (chunk *c);
