#pragma once
#include "../game/character.h"
#include "../voxel/chungus.h"

#include <stdint.h>


#define WORLD_SIZE (16*16*256)

typedef struct {
	chungus *chungi[256][128][256];
} bigchungus;

void        bigchungusInit          (bigchungus *c);
void        bigchungusFree          (bigchungus *c);
void        bigchungusBox           (bigchungus *c, int x,int y,int z, int w,int h,int d,uint8_t block);
void        bigchungusBoxSphere     (bigchungus *c, int x,int y,int z, int r, uint8_t block);
void        bigchungusFreeFarChungi (bigchungus *c, character *cam);
chungus    *bigchungusGetChungus    (bigchungus *c, int x, int y, int z);
uint8_t     bigchungusGetB          (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB          (bigchungus *c, int x, int y, int z, uint8_t b);
void        bigchungusDraw          (bigchungus *c, character *cam);
void        bigchungusGenSpawn      (bigchungus *c);
void        bigchungusGetSpawnPos   (bigchungus *c, int *x, int *y, int *z);

extern bigchungus world;
inline void        worldBox           (int x, int y,int z, int w,int h,int d,uint8_t block){
	bigchungusBox(&world,x,y,z,w,h,d,block);
}
inline void        worldBoxSphere     (int x, int y,int z, int r,uint8_t block){
	bigchungusBoxSphere(&world,x,y,z,r,block);
}
inline uint8_t     worldGetB          (int x, int y, int z){
	return bigchungusGetB(&world,x,y,z);
}
inline chungus*    worldGetChungus    (int x, int y, int z){
	return bigchungusGetChungus(&world,x,y,z);
}
inline bool        worldSetB(int x, int y, int z, uint8_t b){
	return bigchungusSetB(&world,x,y,z,b);
}
inline void        worldSetChungusLoaded(int x, int y, int z){
	chungus *chng = bigchungusGetChungus(&world,x>>8,y>>8,z>>8);
	if(chng != NULL){chng->loaded = 1;}
}

inline int checkCollision(int cx, int cy, int cz){
	return worldGetB(cx,cy,cz) != 0;
}
