#pragma once
#include "../stdint.h"

extern u64 RNGValue;

void  seedRNG   (u64 seed);
u64   getRNGSeed();
u64   rngValR   ();
float rngValf   ();
u64   rngValA   (u64 mask);
u64   rngValM   (u64 max);
i64   rngValMM  (i64 min,i64 max);
