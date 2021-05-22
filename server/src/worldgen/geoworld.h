#pragma once
#include "worldgen.h"

void worldgenGeoSphere     (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenGeoRoundPrism (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenGeoPrism      (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenGeoPyramid    (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenGeoCube       (worldgen *wgen, int x, int y, int z, int size, int b, int fb);
void worldgenGeoIsland  (worldgen *wgen, int x, int y, int z, int size);
