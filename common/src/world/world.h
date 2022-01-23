#pragma once
#include "../common.h"

void     worldBox           (int x, int y, int z, int w, int h, int d, u8 block);
void     worldBoxSphere     (int x, int y, int z, int r, u8 block);
blockId  worldTryB          (int x, int y, int z);
blockId  worldGetB          (int x, int y, int z);
bool     worldSetB          (int x, int y, int z, blockId block);
int      checkCollision     (int x, int y, int z);
chungus *worldTryChungus    (int x, int y, int z);
chungus *worldGetChungus    (int x, int y, int z);
chunk   *worldTryChunk      (int x, int y, int z);
chunk   *worldGetChunk      (int x, int y, int z);
bool     worldIsLoaded      (int x, int y, int z);
vec      chungusGetPos      (const chungus *c);
void     worldMine          (int x, int y, int z);
void     worldBreak         (int x, int y, int z);
void     worldBoxMine       (int x, int y, int z, int w,int h,int d);
void     worldBoxMineSphere (int x, int y, int z, int r);
bool     worldShouldBeLoaded(const vec cpos);

u8       worldTryFluid      (int x, int y, int z);
u8       worldGetFluid      (int x, int y, int z);
bool     worldSetFluid      (int x, int y, int z, u8 level);

u8       worldTryFire       (int x, int y, int z);
u8       worldGetFire       (int x, int y, int z);
bool     worldSetFire       (int x, int y, int z, u8 strength);

void     chunkDirtyRegion   (int x, int y, int z);
