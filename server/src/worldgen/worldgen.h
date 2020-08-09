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



void worldgenFindSpawn  (worldgen *wgen,int x,int z,int tries);
void worldgenMonolith   (worldgen *wgen,int x,int y,int z);
void worldgenObelisk    (worldgen *wgen,int x,int y,int z);

worldgen *worldgenNew(chungus *clay);
void worldgenFree(worldgen *wgen);
void worldgenGenerate(worldgen *wgen);
