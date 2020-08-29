#pragma once
#include "../../../common/src/common.h"

#define CHUNK_SIZE (16)
extern const float CHUNK_RENDER_DISTANCE;
extern const float CHUNK_FADEOUT_DISTANCE;

typedef struct {
	uint16_t x,y,z;
	uint64_t clientsUpdated;
	void *nextFree;
	uint8_t data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
} chunk;

chunk       *chunkNew        (uint16_t x,uint16_t y,uint16_t z);
void         chunkFree       (chunk *c);
void         chunkFill       (chunk *c, uint8_t b);
void         chunkBox        (chunk *c, int x, int y, int z,int gx,int gy,int gz,uint8_t b);
void         chunkGetB       (chunk *c, int x, int y, int z);
void         chunkSetB       (chunk *c, int x, int y, int z, uint8_t b);
int          chunkIsUpdated  (chunk *c, int p);
void         chunkSetUpdated (chunk *c, int p);
unsigned int chunkGetFree    ();
unsigned int chunkGetActive  ();
