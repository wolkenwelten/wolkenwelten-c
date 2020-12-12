#pragma once
#include "../../../common/src/common.h"

#define CHUNK_SIZE (16)

struct chunk {
	u16 x,y,z,dataCount;
	union{
		void *nextFree;
		uint vbo;
	};
	u8 data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

void    chunkInit                  ();
chunk  *chunkNew                   (u16 x,u16 y,u16 z);
void    chunkFree                  (chunk *c);
void    chunkBox                   (chunk *c, int x, int y, int z,int gx,int gy,int gz,u8 b);
void    chunkSetB                  (chunk *c, int x, int y, int z, u8 b);
void    chunkDraw                  (chunk *c, float d);
uint    chunkGetFree               ();
uint    chunkGetActive             ();
uint    chunkGetGeneratedThisFrame ();
void    chunkResetCounter          ();
