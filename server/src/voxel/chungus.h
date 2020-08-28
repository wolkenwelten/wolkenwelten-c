#pragma once
#include "../../../common/src/common.h"
#include "../game/entity.h"
#include "../voxel/chunk.h"

#define CHUNGUS_SIZE (16*16)

typedef struct {
	int x,y,z;
	int spawnx,spawny,spawnz;
	void *nextFree;
	chunk *chunks[16][16][16];
} chungus;

chungus     *chungusNew            (int x,int y, int z);
void         chungusFree           (chungus *c);
void         chungusRoughBox       (chungus *c, int x, int y, int z, int w, int h, int d, uint8_t block);
void         chungusRandomBox      (chungus *c, int x, int y, int z, int w, int h, int d, uint8_t block, int chance);
void         chungusBox            (chungus *c, int x, int y, int z, int w, int h, int d, uint8_t block);
void         chungusBoxF           (chungus *c, int x, int y, int z, int w, int h, int d, uint8_t block);
void         chungusFill           (chungus *c, int x, int y, int z, uint8_t block);
void         chungusSetB           (chungus *c, int x, int y, int z, uint8_t block);
uint8_t      chungusGetB           (chungus *c, int x, int y, int z);
chunk       *chungusGetChunk       (chungus *c, int x, int y, int z);
void         chungusSetSpawn       (chungus *c, int x, int y, int z);
int          chungusGetHighestP    (chungus *c, int x, int *retY, int z);
chungus     *chungusGetActive      (unsigned int i);
void         chungusSetActiveCount (unsigned int i);
unsigned int chungusGetActiveCount ();
