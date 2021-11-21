#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"

#define CHUNK_COUNT (1<<17)

#define CHUNK_SIZE_BITS (4)
#define CHUNK_SIZE (1<<CHUNK_SIZE_BITS)

#define CHUNK_FLAG_DIRTY 1

typedef struct queueEntry queueEntry;

struct chunk {
	u16 x,y,z;
	u8 flags;
	u8 fadeIn;
	struct chunkvertbuf *vertbuf;
	void *nextFree;
	beingList bl;
	blockId data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

void    chunkInit                  ();
chunk  *chunkNew                   (u16 x,u16 y,u16 z);
void    chunkFree                  (chunk *c);
void    chunkDrawQueue             (queueEntry *queue, int queueLen, const vec sideTints[sideMAX]);
void    chunkBox                   (chunk *c, u16 x, u16 y, u16 z,u16 gx,u16 gy,u16 gz,blockId b);
void    chunkSetB                  (chunk *c, u16 x, u16 y, u16 z, blockId b);
uint    chunkGetFree               ();
uint    chunkGetActive             ();
uint    chunkGetGeneratedThisFrame ();
void    chunkResetCounter          ();
void    chunkRecvUpdate            (const packet *p);
