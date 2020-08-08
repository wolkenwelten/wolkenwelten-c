#pragma once
#include<stdint.h>

void     seedRNG(uint64_t seed);
uint64_t getRNGSeed();
float    rngValf();
uint64_t rngValR();
uint64_t rngValM(uint64_t max);
int64_t  rngValMM(int64_t min,int64_t max);
void    *loadFile(char *filename,size_t *len);