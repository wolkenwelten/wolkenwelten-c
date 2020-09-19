#pragma once
#include "../../../common/src/common.h"
#include "../game/entity.h"
#include "../voxel/chunk.h"

#define CHUNGUS_SIZE (16*16)

struct chungus {
	int    x,y,z;
	int    spawnx,spawny,spawnz;
	u64    clientsSubscribed;
	u64    clientsUpdated;
	void  *nextFree;
	chunk *chunks[16][16][16];
};

chungus     *chungusNew              (int x,int y, int z);
void         chungusFree             (chungus *c);
void         chungusLoad             (chungus *c);
void         chungusSave             (chungus *c);
const u8    *chunkLoad               (chungus *c, const u8 *b);
void         chungusRoughBox         (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void         chungusRandomBox        (chungus *c, int x, int y, int z, int w, int h, int d, u8 block, int chance);
void         chungusBox              (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void         chungusBoxF             (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void         chungusFill             (chungus *c, int x, int y, int z, u8 block);
void         chungusSetB             (chungus *c, int x, int y, int z, u8 block);
u8           chungusGetB             (chungus *c, int x, int y, int z);
chunk       *chungusGetChunk         (chungus *c, int x, int y, int z);
void         chungusSetSpawn         (chungus *c, int x, int y, int z);
void         chungusSubscribePlayer  (chungus *c, uint p);
int          chungusUnsubscribePlayer(chungus *c, uint p);
uint         chungusIsSubscribed     (chungus *c, uint p);
int          chungusUpdateClient     (chungus *c, uint p);
uint         chungusIsUpdated        (chungus *c, uint p);
void         chungusSetUpdated       (chungus *c, uint p);
int          chungusGetHighestP      (chungus *c, int x, int *retY, int z);
chungus     *chungusGetActive        (uint i);
void         chungusSetActiveCount   (uint i);
uint         chungusGetActiveCount   ();
