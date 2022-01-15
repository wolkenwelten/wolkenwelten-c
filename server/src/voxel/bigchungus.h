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
void        bigchungusBox               (bigchungus *c, int x, int y, int z, int w,int h,int d,blockId block);
void        bigchungusBoxSphere         (bigchungus *c, int x, int y, int z, int r, blockId block);
void        bigchungusMine              (bigchungus *c, int x, int y, int z);
void        bigchungusBoxMine           (bigchungus *c, int x, int y, int z, int w,int h,int d);
void        bigchungusBoxMineSphere     (bigchungus *c, int x, int y, int z, int r);
chungus    *bigchungusTryChungus        (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus        (bigchungus *c, int x, int y, int z);
void        bigchungusFreeChungus       (bigchungus *c, int x, int y, int z);
chunk      *bigchungusTryChunk          (bigchungus *c, int x, int y, int z);
chunk      *bigchungusGetChunk          (bigchungus *c, int x, int y, int z);
bool        bigchungusGetHighestP       (bigchungus *c, int x, int *rety, int z);
blockId     bigchungusTryB              (bigchungus *c, int x, int y, int z);
blockId     bigchungusGetB              (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB              (bigchungus *c, int x, int y, int z, blockId b);
void        bigchungusGenSpawn          (bigchungus *c);
void        bigchungusGenHugeSpawn      (bigchungus *c);
vec         bigchungusGetSpawnPos       (bigchungus *c);
void        bigchungusSetSpawnPos       (bigchungus *c, vec pos);
void        bigchungusUpdateClient      (bigchungus *c, int p);
void        bigchungusUnsubscribeClient (bigchungus *c, int p);
void        bigchungusDirtyChunk        (bigchungus *c, int x, int y, int z, int client);

void     worldBox           (int x, int y, int z, int w, int h, int d, blockId block);
void     worldBoxSphere     (int x, int y, int z, int r, blockId block);
blockId  worldTryB          (int x, int y, int z);
blockId  worldGetB          (int x, int y, int z);
chungus *worldTryChungus    (int x, int y, int z);
chungus *worldTryChungusV   (const vec pos);
chunk   *worldTryChunk      (int x, int y, int z);
chungus *worldGetChungus    (int x, int y, int z);
chunk   *worldGetChunk      (int x, int y, int z);
bool     worldSetB          (int x, int y, int z, blockId block);
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

bool     bigchungusSetFluid(bigchungus *c, int x, int y, int z, int level);
u8       bigchungusTryFluid(bigchungus *c, int x, int y, int z);
u8       bigchungusGetFluid(bigchungus *c, int x, int y, int z);
u8       worldTryFluid(int x, int y, int z);
u8       worldGetFluid(int x, int y, int z);
bool     worldSetFluid(int x, int y, int z, int level);

bool     bigchungusSetFire(bigchungus *c, int x, int y, int z, int strength);
u8       bigchungusTryFire(bigchungus *c, int x, int y, int z);
u8       bigchungusGetFire(bigchungus *c, int x, int y, int z);
u8       worldTryFire(int x, int y, int z);
u8       worldGetFire(int x, int y, int z);
bool     worldSetFire(int x, int y, int z, int level);
