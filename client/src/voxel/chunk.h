#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"

#define CHUNK_FLAG_DIRTY 1

typedef struct queueEntry queueEntry;

struct chunk {
	u16 x,y,z;
	u8 flags;
	u8 fadeIn;

	struct chunkvertbuf *vertbuf;
	beingList bl;

	chunkOverlay *fluid, *block, *fire;
};

void    chunkInit                  ();
void    chunkReset                 (chunk *c, int x, int y, int z);
void    chunkFree                  (chunk *c);
void    chunkDrawQueue             (queueEntry *queue, int queueLen, const vec sideTints[sideMAX]);
void    chunkBox                   (chunk *c, u16 x, u16 y, u16 z,u16 gx,u16 gy,u16 gz,blockId b);
void    chunkSetB                  (chunk *c, u16 x, u16 y, u16 z, blockId b);
uint    chunkGetGeneratedThisFrame ();
void    chunkResetCounter          ();
void    chunkRecvUpdate            (const packet *p);
