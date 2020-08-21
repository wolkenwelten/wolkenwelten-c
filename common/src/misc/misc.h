#pragma once
#include <stdint.h>
#include <stdlib.h>

void        seedRNG(uint64_t seed);
uint64_t    getRNGSeed();
float       rngValf();
uint64_t    rngValR();
uint64_t    rngValM(uint64_t max);
int64_t     rngValMM(int64_t min,int64_t max);
float       animationInterpolation(int left, int max , float midPoint);
float       animationInterpolationSustain(int left, int max , float startPoint, float stopPoint);
void       *loadFile(char *filename,size_t *len);
const char *getHumanReadableSize(size_t n);
char      **splitArgs(const char *cmd,int *rargc);
