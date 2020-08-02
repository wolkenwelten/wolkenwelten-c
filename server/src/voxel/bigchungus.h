#pragma once

#include "../game/entity.h"
#include "../voxel/chungus.h"

#include <stdint.h>

#define WORLD_SIZE (16*16*256)

typedef struct {
	int spawnx, spawny, spawnz;
	chungus *chungi[256][128][256];
	unsigned char vegetationConcentration[256][256];
	unsigned char islandSizeModifier[256][256];
	unsigned char islandCountModifier[256][256];
	unsigned char geoworldMap[256][256];

} bigchungus;

void        bigchungusInit          (bigchungus *c);
void        bigchungusFree          (bigchungus *c);
void        bigchungusBox           (bigchungus *c, int x,int y,int z, int w,int h,int d,uint8_t block);
void        bigchungusBoxMine       (bigchungus *c, int x,int y,int z, int w,int h,int d);
void        bigchungusBoxMineSphere (bigchungus *c, int x,int y,int z, int r);
chunk      *bigchungusGetChunk      (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus    (bigchungus *c, int x, int y, int z);
bool        bigchungusGetHighestP   (bigchungus *c, int x, int *rety, int z);
uint8_t     bigchungusGetB          (bigchungus *c, int x, int y, int z);
void        bigchungusSetB          (bigchungus *c, int x, int y, int z, uint8_t b);
void        bigchungusGenSpawn      (bigchungus *c);
void        bigchungusGetSpawnPos   (bigchungus *c, int *x, int *y, int *z);
void        bigchungusFreeFarChungi (bigchungus *c);

extern bigchungus world;
inline void        worldBox           (int x, int y, int z, int w,int h,int d,uint8_t block){
	bigchungusBox(&world,x,y,z,w,h,d,block);
}
inline void        worldBoxMine       (int x, int y, int z, int w,int h,int d){
	bigchungusBoxMine(&world,x,y,z,w,h,d);
}
inline void        worldBoxMineSphere (int x, int y, int z, int r){
	bigchungusBoxMineSphere(&world,x,y,z,r);
}
inline uint8_t     worldGetB          (int x, int y, int z){
	return bigchungusGetB(&world,x,y,z);
}
inline void        worldSetB          (int x, int y, int z, uint8_t b){
	bigchungusSetB(&world,x,y,z,b);
}
inline chungus*    worldGetChungus    (int x, int y, int z){
	return bigchungusGetChungus(&world,x,y,z);
}
inline chunk*      worldGetChunk      (int x, int y, int z){
	return bigchungusGetChunk(&world,x,y,z);
}
inline void        worldGetSpawnPos   (int *x, int *y, int *z){
	bigchungusGetSpawnPos(&world,x,y,z);
}
inline bool checkCollision(int cx, int cy, int cz){
	return worldGetB(cx,cy,cz) != 0;
}
