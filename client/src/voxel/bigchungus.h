#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chungus.h"

#define WORLD_SIZE (16*16*256)

struct bigchungus {
	chungus *chungi[256][128][256];
};

void        bigchungusInit          (bigchungus *c);
void        bigchungusFree          (bigchungus *c);
void        bigchungusBox           (bigchungus *c, int x,int y,int z, int w,int h,int d,u8 block);
void        bigchungusBoxSphere     (bigchungus *c, int x,int y,int z, int r, u8 block);
void        bigchungusFreeFarChungi (bigchungus *c, character *cam);
chungus    *bigchungusTryChungus    (bigchungus *c, int x, int y, int z);
chungus    *bigchungusGetChungus    (bigchungus *c, int x, int y, int z);
chunk      *bigchungusGetChunk      (bigchungus *c, int x, int y, int z);
u8          bigchungusGetB          (bigchungus *c, int x, int y, int z);
bool        bigchungusSetB          (bigchungus *c, int x, int y, int z, u8 b);
void        bigchungusDraw          (bigchungus *c, const character *cam);

extern bigchungus world;

void     worldBox              (int x, int y, int z, int w, int h, int d, u8 block);
void     worldBoxSphere        (int x, int y, int z, int r, u8 block);
u8       worldGetB             (int x, int y, int z);
chungus* worldTryChungus       (int x, int y, int z);
chungus* worldGetChungus       (int x, int y, int z);
chunk*   worldGetChunk         (int x, int y, int z);
bool     worldSetB             (int x, int y, int z, u8 block);
void     worldSetChungusLoaded (int x, int y, int z);
int      checkCollision        (int x, int y, int z);
