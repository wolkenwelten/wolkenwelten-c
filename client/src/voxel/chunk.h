#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"

#define CHUNK_SIZE_BITS (4)
#define CHUNK_SIZE (1<<CHUNK_SIZE_BITS)

#define CHUNK_FLAG_DIRTY 1

struct chunk {
	u16 x,y,z,vboSize;
	u16 sideEnd[sideMAX];
	u8 flags;
	u8 fadeIn;
	void *nextFree;
	uint vao,vbo;
	beingList bl;
	u8 data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

void    chunkInit                  ();
chunk  *chunkNew                   (u16 x,u16 y,u16 z);
void    chunkFree                  (chunk *c);
void    chunkBox                   (chunk *c, u16 x, u16 y, u16 z,u16 gx,u16 gy,u16 gz,u8 b);
void    chunkSetB                  (chunk *c, u16 x, u16 y, u16 z, u8 b);
void    chunkDraw                  (chunk *c, float d, sideMask mask, const vec sideTints[6]);
uint    chunkGetFree               ();
uint    chunkGetActive             ();
uint    chunkGetGeneratedThisFrame ();
void    chunkResetCounter          ();
void    chunkRecvUpdate            (const packet *p);
