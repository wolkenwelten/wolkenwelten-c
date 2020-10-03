#pragma once
#include "../../../common/src/common.h"

#define WORLD_SIZE (16*16*256)

struct bigchungus {
	ivec spawn;
	chungus *chungi[256][128][256];
	u8 heightModifier          [256][256];
	u8 vegetationConcentration [256][256];
	u8 islandSizeModifier      [256][256];
	u8 islandCountModifier     [256][256];
	u8 geoworldMap             [256][256];

};

void        bigchungusInit              (bigchungus *c);
void        bigchungusFree              (bigchungus *c);
void        bigchungusBox               (bigchungus *c, int x, int y, int z, int w,int h,int d,u8 block);
void        bigchungusBoxSphere         (bigchungus *c, int x, int y, int z, int r, u8 block);
void        bigchungusBoxMine           (bigchungus *c, int x, int y, int z, int w,int h,int d);
void        bigchungusBoxMineSphere     (bigchungus *c, int x, int y, int z, int r);
chungus    *bigchungusTryChungus        (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus        (bigchungus *c, int x, int y, int z);
chunk      *bigchungusTryChunk          (bigchungus *c, int x, int y, int z);
chunk      *bigchungusGetChunk          (bigchungus *c, int x, int y, int z);
bool        bigchungusGetHighestP       (bigchungus *c, int x, int *rety, int z);
u8          bigchungusGetB              (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB              (bigchungus *c, int x, int y, int z, u8 b);
void        bigchungusGenSpawn          (bigchungus *c);
ivec        bigchungusGetSpawnPos       (bigchungus *c);
void        bigchungusFreeFarChungi     (bigchungus *c);
void        bigchungusUpdateClient      (bigchungus *c, int p);
void        bigchungusUnsubscribeClient (bigchungus *c, int p);
void        bigchungusDirtyChunk        (bigchungus *c, int x, int y, int z, int client);

extern bigchungus world;

inline void        worldBoxMine       (int x, int y, int z, int w,int h,int d){
	bigchungusBoxMine(&world,x,y,z,w,h,d);
}
inline void        worldBoxMineSphere (int x, int y, int z, int r){
	bigchungusBoxMineSphere(&world,x,y,z,r);
}
inline ivec        worldGetSpawnPos   (){
	return bigchungusGetSpawnPos(&world);
}


void     worldBox          (int x, int y, int z, int w, int h, int d, u8 block);
void     worldBoxSphere    (int x, int y, int z, int r, u8 block);
u8       worldGetB         (int x, int y, int z);
chungus* worldTryChungus   (int x, int y, int z);
chungus* worldGetChungus   (int x, int y, int z);
chunk*   worldGetChunk     (int x, int y, int z);
bool     worldSetB         (int x, int y, int z, u8 block);
int      checkCollision    (int x, int y, int z);
void     worldDirtyChunk   (int x, int y, int z, int client);
