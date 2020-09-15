#pragma once

void generateNoise      (unsigned int seed, unsigned char heightmap[256][256]);
void generateNoiseZoomed(unsigned int seed, unsigned char heightmap[256][256], int x, int y, unsigned char parent[256][256]);
