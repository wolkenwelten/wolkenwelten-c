#pragma once
#include "../../../common/src/common.h"
#include "../voxel/chungus.h"

void     worldFree             ();
void     worldFreeFarChungi    (const character *cam);
void     worldDraw             (const character *cam);
void     worldBox              (int x, int y, int z, int w,int h,int d,blockId block);
void     worldBoxSphere        (int x, int y, int z, int r,blockId block);
void     worldBoxSphereDirty   (int x, int y, int z, int r);
blockId  worldTryB             (int x, int y, int z);
blockId  worldGetB             (int x, int y, int z);
bool     worldSetB             (int x, int y, int z, blockId block);
chungus *worldTryChungus       (int x, int y, int z);
chungus *worldGetChungus       (int x, int y, int z);
chunk   *worldGetChunk         (int x, int y, int z);
chunk   *worldTryChunk         (int x, int y, int z);
void     worldSetChungusLoaded (int x, int y, int z);
void     worldMine             (int x, int y, int z);
void     worldBreak            (int x, int y, int z);
void     worldBoxMine          (int x, int y, int z, int w, int h, int d);
void     worldBoxMineSphere    (int x, int y, int z, int r);
bool     worldIsLoaded         (int x, int y, int z);
void     worldSetChunkUpdated  (int x, int y, int z);
bool     worldShouldBeLoaded   (const vec cpos);

u8       worldTryFluid         (int x, int y, int z);
u8       worldGetFluid         (int x, int y, int z);
bool     worldSetFluid         (int x, int y, int z, u8 level);

u8       worldTryFire          (int x, int y, int z);
u8       worldGetFire          (int x, int y, int z);
bool     worldSetFire          (int x, int y, int z, u8 strength);

u8       worldTryLight         (int x, int y, int z);

int      checkCollision        (int x, int y, int z);
