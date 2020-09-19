#pragma once
#include "../stdint.h"
#include <stdlib.h>

void        seedRNG(u64 seed);
u64         getRNGSeed();
float       rngValf();
u64         rngValR();
u64         rngValM(u64 max);
i64         rngValMM(i64 min,i64 max);
float       animationInterpolation(int left, int max , float midPoint);
float       animationInterpolationSustain(int left, int max , float startPoint, float stopPoint);
void        saveFile(const char *filename,const void *buf, size_t len);
void       *loadFile(const char *filename,size_t *len);
const char *getHumanReadableSize(size_t n);
char      **splitArgs(const char *cmd,int *rargc);
int         isDir    (const char *name);
int         isFile   (const char *name);
void        makeDir  (const char *name);
