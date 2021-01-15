#pragma once
#include "../../../common/src/common.h"

#define CHUNK_SIZE (16)

struct chunk {
	u16 x,y,z,dataCount;
	void *nextFree;
	uint vbo;
	uint vao;
	beingList bl;
	u8 data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

void    chunkInit                  ();
chunk  *chunkNew                   (u16 x,u16 y,u16 z);
void    chunkFree                  (chunk *c);
void    chunkBox                   (chunk *c, u16 x, u16 y, u16 z,u16 gx,u16 gy,u16 gz,u8 b);
void    chunkSetB                  (chunk *c, u16 x, u16 y, u16 z, u8 b);
void    chunkDraw                  (chunk *c, float d);
uint    chunkGetFree               ();
uint    chunkGetActive             ();
uint    chunkGetGeneratedThisFrame ();
void    chunkResetCounter          ();
void    chunkRecvUpdate            (const packet *p);
