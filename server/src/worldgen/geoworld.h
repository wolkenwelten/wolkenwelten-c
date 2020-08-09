#pragma once
#include "worldgen.h"

void worldgenSphere     (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenRoundPrism (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenPrism      (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenPyramid    (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenCube       (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenGeoIsland  (worldgen *wgen, int x, int y, int z, int size);