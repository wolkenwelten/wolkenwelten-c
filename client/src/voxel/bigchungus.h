#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chungus.h"

void     worldFree             ();
void     worldFreeFarChungi    (const character *cam);
void     worldDraw             (const character *cam);
void     worldBox              (int x, int y, int z, int w,int h,int d,u8 block);
void     worldBoxSphere        (int x, int y, int z, int r,u8 block);
void     worldBoxSphereDirty   (int x, int y, int z, int r);
u8       worldTryB             (int x, int y, int z);
u8       worldGetB             (int x, int y, int z);
chungus *worldTryChungus       (int x, int y, int z);
chungus *worldGetChungus       (int x, int y, int z);
chunk   *worldGetChunk         (int x, int y, int z);
chunk   *worldTryChunk         (int x, int y, int z);
bool     worldSetB             (int x, int y, int z, u8 block);
void     worldSetChungusLoaded (int x, int y, int z);
void     worldBoxMine          (int x, int y, int z, int w, int h, int d);
int      checkCollision        (int x, int y, int z);
bool     worldIsLoaded         (int x, int y, int z);
