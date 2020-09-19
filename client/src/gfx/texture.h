#pragma once
#include "../../../common/src/common.h"

texture *textureNew               (const u8 *data, size_t dataLen,const char *filename);
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
extern texture *tBlockMining;
