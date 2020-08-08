#pragma once
#include <stddef.h>

typedef struct {
	unsigned int ID,w,h;
	char *filename;
} texture;

texture *textureNew (const unsigned char *data, size_t dataLen, char *filename);
void     textureFree();
void     textureBind(const texture *tex);
void     textureInit();
void     textureBuildBlockIcons(int loadFromFile);
void     textureReload();

extern texture *tBlocks;
extern texture *tGui;
extern texture *tCursor;
extern texture *tCrosshair;
extern texture *tRope;
extern texture *tBlockMining;

