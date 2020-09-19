#pragma once
#include "../common.h"

void generateNoise      (u64 seed, u8 heightmap[256][256]);
void generateNoiseZoomed(u64 seed, u8 heightmap[256][256], uint x, uint y, u8 parent[256][256]);
