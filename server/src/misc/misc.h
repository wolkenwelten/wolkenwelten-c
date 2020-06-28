#pragma once
#include <stdint.h>

void         seedRNG(unsigned int seed);
unsigned int getTicks();
unsigned int getRNGSeed();
float        rngValf();
unsigned int rngValR();
unsigned int rngValM(unsigned int max);
int          rngValMM(int min,int max);
uint64_t     getMillis();
const char  *base64_encode(const unsigned char *data,unsigned int input_length,const char *prefix);
