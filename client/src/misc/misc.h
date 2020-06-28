#pragma once

void         seedRNG(unsigned int seed);
unsigned int getRNGSeed();
float        rngValf();
unsigned int rngValR();
unsigned int rngValM(unsigned int max);
int          rngValMM(int min,int max);
