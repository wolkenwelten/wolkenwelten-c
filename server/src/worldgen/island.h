#pragma once
#include "worldgen.h"

void worldgenRemoveDirt (worldgen *wgen);
void worldgenRock       (worldgen *wgen,int x,int y,int z,int w,int h,int d);
void worldgenIsland     (worldgen *wgen,int x,int y,int z,int size);
void worldgenSCluster   (worldgen *wgen,int x,int y,int z,int size, int csize);
void worldgenCluster    (worldgen *wgen,int size, int iSize, int iMin,int iMax);
void worldgenSRock      (worldgen *wgen,int x,int y,int z,int w, int h, int d);
void worldgenCRock      (worldgen *wgen,int x,int y,int z,int w, int h, int d);
void worldgenCCluster   (worldgen *wgen,int size, int count);