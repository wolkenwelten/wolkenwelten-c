#pragma once
#include "../stdint.h"
#include <stdlib.h>

float       animationInterpolation(int left, int max , float midPoint);
float       animationInterpolationSustain(int left, int max , float startPoint, float stopPoint);
void        saveFile(const char *filename,const void *buf, size_t len);
void       *loadFile(const char *filename,size_t *len);
const char *getHumanReadableSize(size_t n);
char      **splitArgs(const char *cmd,int *rargc);
int         isDir    (const char *name);
int         isFile   (const char *name);
void        makeDir  (const char *name);
void        rmDirR   (const char *name);

static inline int inWorld(uint x, uint y, uint z){
	return ((y&~0x7FFF) | ((x|z)&~0xFFFF))==0;
}
