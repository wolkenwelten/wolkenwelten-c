#pragma once
#include "../stdint.h"
#include <stdlib.h>

float       animationInterpolation(int left, int max , float midPoint);
float       animationInterpolationSustain(int left, int max , float startPoint, float stopPoint);
u32         colorInterpolate(u32 c1, u32 c2, float i);

void        saveFile(const char *filename,const void *buf, size_t len);
void       *loadFile(const char *filename,size_t *len);

int         parseAnsiCode(const char *str, int *fgc, int *bgc);
const char *getHumanReadableSize(size_t n);
char      **splitArgs(const char *cmd,int *rargc);

int         isDir    (const char *name);
int         isFile   (const char *name);
void        makeDir  (const char *name);
void        rmDirR   (const char *name);

static inline int inWorld(uint x, uint y, uint z){
	return ((y&~0x7FFF) | ((x|z)&~0xFFFF))==0;
}

u64 SHA1Simple(const void *data, uint len);
