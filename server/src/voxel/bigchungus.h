#pragma once
#include "../../../common/src/common.h"
#include "../game/entity.h"
#include "../voxel/chungus.h"

#define WORLD_SIZE (16*16*256)

struct bigchungus {
	int spawnx, spawny, spawnz;
	chungus *chungi[256][128][256];
	unsigned char heightModifier          [256][256];
	unsigned char vegetationConcentration [256][256];
	unsigned char islandSizeModifier      [256][256];
	unsigned char islandCountModifier     [256][256];
	unsigned char geoworldMap             [256][256];

};

void        bigchungusInit              (bigchungus *c);
void        bigchungusFree              (bigchungus *c);
void        bigchungusBox               (bigchungus *c, int x, int y, int z, int w,int h,int d,uint8_t block);
void        bigchungusBoxSphere         (bigchungus *c, int x, int y, int z, int r, uint8_t block);
void        bigchungusBoxMine           (bigchungus *c, int x, int y, int z, int w,int h,int d);
void        bigchungusBoxMineSphere     (bigchungus *c, int x, int y, int z, int r);
chunk      *bigchungusGetChunk          (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus        (bigchungus *c, int x, int y, int z);
bool        bigchungusGetHighestP       (bigchungus *c, int x, int *rety, int z);
uint8_t     bigchungusGetB              (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB              (bigchungus *c, int x, int y, int z, uint8_t b);
void        bigchungusGenSpawn          (bigchungus *c);
void        bigchungusGetSpawnPos       (bigchungus *c, int *x, int *y, int *z);
void        bigchungusFreeFarChungi     (bigchungus *c);
void        bigchungusUpdateClient      (bigchungus *c, int p);
void        bigchungusUnsubscribeClient (bigchungus *c, int p);

extern bigchungus world;

inline void        worldBoxMine       (int x, int y, int z, int w,int h,int d){
	bigchungusBoxMine(&world,x,y,z,w,h,d);
}
inline void        worldBoxMineSphere (int x, int y, int z, int r){
	bigchungusBoxMineSphere(&world,x,y,z,r);
}
inline void        worldGetSpawnPos   (int *x, int *y, int *z){
	bigchungusGetSpawnPos(&world,x,y,z);
}


void     worldBox              (int x, int y, int z, int w, int h, int d, uint8_t block);
void     worldBoxSphere        (int x, int y, int z, int r, uint8_t block);
uint8_t  worldGetB             (int x, int y, int z);
chungus* worldGetChungus       (int x, int y, int z);
chunk*   worldGetChunk         (int x, int y, int z);
bool     worldSetB             (int x, int y, int z, uint8_t block);
int      checkCollision        (int x, int y, int z);
