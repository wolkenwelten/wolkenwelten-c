#pragma once
#include "../../../common/src/common.h"

texture *textureNewRaw            ();
texture *textureNew               (const u8 *data, uint dataLen,const char *filename);
texture *textureNewArray          (const u8 *data, uint dataLen,const char *filename, int d);
void     textureLoadSurface       (texture *t, uint w, uint h, const void *data, bool linear);
void     textureFree              ();
void     textureBind              (const texture *tex);
void     textureInit              ();
void     textureBuildBlockIcons   (int loadFromFile);
void     textureReload            ();
void     checkTexturesForReloading();

extern texture *tBlocks;
extern texture *tGui;
extern texture *tCursor;
extern texture *tCrosshair;
extern texture *tRope;
extern texture *tSteelrope;
extern texture *tBlockMining;
extern texture *tBlocksArr;
extern texture *tWolkenwelten;
