#pragma once
#include "../voxel/chungus.h"

typedef struct {
	chungus *clay;
	int gx,gy,gz;
	int layer;
	int minX,minY,minZ;
	int maxX,maxY,maxZ;
	int vegetationChance;
	int jaggyness;
	int geoworld;

	unsigned char vegetationConcentration;
	unsigned char islandSizeModifier;
	unsigned char islandCountModifier;

	int iterChance;
	bool geoIslands;
	bool geoIslandChance;

	void *nextFree;
} worldgen;


void worldgenRemoveDirt (worldgen *wgen);
void worldgenFindSpawn  (worldgen *wgen,int x,int z,int tries);
void worldgenMonolith   (worldgen *wgen,int x,int y,int z);
void worldgenObelisk    (worldgen *wgen,int x,int y,int z);

void worldgenShrub      (worldgen *wgen,int x,int y,int z);
void worldgenDeadTree   (worldgen *wgen,int x,int y,int z);
void worldgenBigDeadTree(worldgen *wgen,int x,int y,int z);
void worldgenSpruce     (worldgen *wgen,int x,int y,int z);
void worldgenBigSpruce  (worldgen *wgen,int x,int y,int z);
void worldgenOak        (worldgen *wgen,int x,int y,int z);
void worldgenBigOak     (worldgen *wgen,int x,int y,int z);
void worldgenRoots      (worldgen *wgen,int x,int y,int z);
void worldgenBigRoots   (worldgen *wgen,int x,int y,int z);

void worldgenRock       (worldgen *wgen,int x,int y,int z,int w,int h,int d);
void worldgenPrism      (worldgen *wgen,int x,int y,int z,int size,int b);
void worldgenPyramid    (worldgen *wgen,int x,int y,int z,int size,int b);
void worldgenIsland     (worldgen *wgen,int x,int y,int z,int size);
void worldgenSCluster   (worldgen *wgen,int x,int y,int z,int size, int csize);
void worldgenCluster    (worldgen *wgen,int size, int iSize, int iMin,int iMax);
void worldgenLabyrinth  (worldgen *wgen,int labLayer);
void worldgenSRock      (worldgen *wgen,int x,int y,int z,int w, int h, int d);
void worldgenCRock      (worldgen *wgen,int x,int y,int z,int w, int h, int d);
void worldgenCCluster   (worldgen *wgen,int size, int count);

worldgen *worldgenNew(chungus *clay);
void worldgenFree(worldgen *wgen);
void worldgenGenerate(worldgen *wgen);
