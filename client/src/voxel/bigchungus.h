#pragma once
#include "../../../common/src/common.h"
#include "../game/character.h"
#include "../voxel/chungus.h"

#define WORLD_SIZE (16*16*256)

struct bigchungus {
	chungus *chungi[256][128][256];
};

void        bigchungusInit          (bigchungus *c);
void        bigchungusFree          (bigchungus *c);
void        bigchungusBox           (bigchungus *c, int x,int y,int z, int w,int h,int d,uint8_t block);
void        bigchungusBoxSphere     (bigchungus *c, int x,int y,int z, int r, uint8_t block);
void        bigchungusFreeFarChungi (bigchungus *c, character *cam);
chungus    *bigchungusTryChungus    (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus    (bigchungus *c, int x, int y, int z);
chunk      *bigchungusGetChunk      (bigchungus *c, int x, int y, int z);
uint8_t     bigchungusGetB          (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB          (bigchungus *c, int x, int y, int z, uint8_t b);
void        bigchungusDraw          (bigchungus *c, character *cam);
void        bigchungusGenSpawn      (bigchungus *c);
void        bigchungusGetSpawnPos   (bigchungus *c, int *x, int *y, int *z);

extern bigchungus world;

void     worldBox              (int x, int y, int z, int w, int h, int d, uint8_t block);
void     worldBoxSphere        (int x, int y, int z, int r, uint8_t block);
uint8_t  worldGetB             (int x, int y, int z);
chungus* worldTryChungus       (int x, int y, int z);
chungus* worldGetChungus       (int x, int y, int z);
chunk*   worldGetChunk         (int x, int y, int z);
bool     worldSetB             (int x, int y, int z, uint8_t block);
void     worldSetChungusLoaded (int x, int y, int z);
int      checkCollision        (int x, int y, int z);

