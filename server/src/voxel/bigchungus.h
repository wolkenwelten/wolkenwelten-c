#pragma once
#include "../../../common/src/common.h"

#define WORLD_SIZE (16*16*256)

struct bigchungus {
	chungus *chungi            [256][128][256];
	u16 sx,sy,sz;
	u8 heightModifier          [256][256];
	u8 vegetationConcentration [256][256];
	u8 islandSizeModifier      [256][256];
	u8 islandCountModifier     [256][256];
	u8 geoworldMap             [256][256];
};
extern bigchungus world;

void        bigchungusInit              (bigchungus *c);
void        bigchungusFree              (bigchungus *c);
void        bigchungusBox               (bigchungus *c, int x, int y, int z, int w,int h,int d,u8 block);
void        bigchungusBoxSphere         (bigchungus *c, int x, int y, int z, int r, u8 block);
void        bigchungusMine              (bigchungus *c, int x, int y, int z);
void        bigchungusBoxMine           (bigchungus *c, int x, int y, int z, int w,int h,int d);
void        bigchungusBoxMineSphere     (bigchungus *c, int x, int y, int z, int r);
chungus    *bigchungusTryChungus        (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus        (bigchungus *c, int x, int y, int z);
void        bigchungusFreeChungus       (bigchungus *c, int x, int y, int z);
chunk      *bigchungusTryChunk          (bigchungus *c, int x, int y, int z);
chunk      *bigchungusGetChunk          (bigchungus *c, int x, int y, int z);
bool        bigchungusGetHighestP       (bigchungus *c, int x, int *rety, int z);
u8          bigchungusTryB              (bigchungus *c, int x, int y, int z);
u8          bigchungusGetB              (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB              (bigchungus *c, int x, int y, int z, u8 b);
void        bigchungusGenSpawn          (bigchungus *c);
void        bigchungusGenHugeSpawn      (bigchungus *c);
vec         bigchungusGetSpawnPos       (bigchungus *c);
void        bigchungusSetSpawnPos       (bigchungus *c, vec pos);
void        bigchungusUpdateClient      (bigchungus *c, int p);
void        bigchungusUnsubscribeClient (bigchungus *c, int p);
void        bigchungusDirtyChunk        (bigchungus *c, int x, int y, int z, int client);

void     worldBox           (int x, int y, int z, int w, int h, int d, u8 block);
void     worldBoxSphere     (int x, int y, int z, int r, u8 block);
u8       worldTryB          (int x, int y, int z);
u8       worldGetB          (int x, int y, int z);
chungus *worldTryChungus    (int x, int y, int z);
chungus *worldTryChungusV   (const vec pos);
chunk   *worldTryChunk      (int x, int y, int z);
chungus *worldGetChungus    (int x, int y, int z);
chunk   *worldGetChunk      (int x, int y, int z);
bool     worldSetB          (int x, int y, int z, u8 block);
int      checkCollision     (int x, int y, int z);
void     worldDirtyChunk    (int x, int y, int z, int client);
void     worldMine          (int x, int y, int z);
void     worldBreak         (int x, int y, int z);
void     worldBoxMine       (int x, int y, int z, int w,int h,int d);
void     worldBoxMineSphere (int x, int y, int z, int r);
bool     worldIsLoaded      (int x, int y, int z);
bool     worldShouldBeLoaded(const vec cpos);
vec      worldGetSpawnPos   ();
void     worldSetSpawnPos   (vec pos);
void     worldSetAllUpdated ();
