#pragma once
#include "../../../common/src/common.h"

#ifndef __EMSCRIPTEN__
#define CHUNK_COUNT (1<<18)
#else
#define CHUNK_COUNT (1<<17)
#endif

extern chunk *chunkList;
extern u8    *chunkData;

struct chunk {
	u16 x,y,z,dataCount;
	union{
		void *nextFree;
		uint vbo;
	};
	uint vao;
	beingList bl;
};

typedef struct {
	u8 data[16][16][16];
} chunkD;

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

static inline u8 *chunkGetRawData  (const chunk *c){
	const uint i = c - chunkList;
	return &chunkData[i * 4096];
}

static inline chunkD *chunkGetData(const chunk *c){
	return (chunkD *)chunkGetRawData(c);
}
