#include "worldgen.h"


#include "../main.h"
#include "../misc/options.h"
#include "../voxel/chunk.h"
#include "../voxel/chungus.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

#include "geoworld.h"
#include "island.h"
#include "labyrinth.h"
#include "landmass.h"
#include "vegetation.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

worldgen worldgenList[4];
int worldgenCount = 0;
worldgen *worldgenFirstFree = NULL;

worldgen *worldgenNew(chungus *nclay){
	worldgen *wgen = NULL;
	if(worldgenFirstFree != NULL){
		wgen = worldgenFirstFree;
		worldgenFirstFree = wgen->nextFree;
	}
	if(wgen == NULL){
		wgen = &worldgenList[worldgenCount++];
	}
	wgen->clay = nclay;
	wgen->iterChance = 64;
	wgen->gx = nclay->x;
	wgen->gy = nclay->y;
	wgen->gz = nclay->z;
	wgen->layer = (wgen->gy / CHUNGUS_SIZE);
	wgen->minX = wgen->minY = wgen->minZ = CHUNGUS_SIZE;
	int gx = wgen->gx >> 8;
	int gy = wgen->gy;
	int gz = wgen->gz >> 8;

	wgen->heightModifier          = world.heightModifier         [gx][gz];
	wgen->vegetationConcentration = world.vegetationConcentration[gx][gz];
	wgen->islandSizeModifier      = world.islandSizeModifier     [gx][gz];
	wgen->islandCountModifier     = world.islandCountModifier    [gx][gz];
	wgen->geoworld                = world.geoworldMap            [gx][gz] + ((gy & 0xF0) << 2);
	wgen->geoIslandChance         = wgen->geoIslands = false;

	return wgen;
}

void worldgenFree(worldgen *wgen){
	wgen->nextFree = worldgenFirstFree;
	worldgenFirstFree = wgen;
}

void worldgenObelisk(worldgen *wgen, int x,int y,int z){
	int size = rngValMM(8,16);
	for(int cy=-4;cy<size;cy++){
		chungusSetB(wgen->clay,x,y+cy,z,12);
	}
}

void worldgenMonolith(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	for(int ox=-3;ox<6;ox++){
		for(int oy=0;oy<12;oy++){
			for(int oz=-3;oz<4;oz++){
				chungusSetB(clay,x+ox,y+oy,z+oz,0);
			}
		}
	}
	for(int ox=-1;ox<4;ox++){
		for(int oy=0;oy<10;oy++){
			chungusSetB(clay,x+ox,y+oy,z,9);
		}
	}
	for(int ox=-3;ox<6;ox++){
		for(int oy=-5;oy<0;oy++){
			for(int oz=-3;oz<4;oz++){
				chungusSetB(clay,x+ox,y+oy,z+oz,3);
			}
		}
	}
}

void worldgenFindSpawn(worldgen *wgen, int x,int z,int tries){
	if(tries > 8){
		wgen->clay->spawn = ivecNOne();
		return;
	}
	int y = 0;
	if(chungusGetHighestP(wgen->clay,x,&y,z)){
		wgen->clay->spawn = ivecNew(x,y,z);
	}else{
		worldgenFindSpawn(wgen,rngValMM(0,CHUNGUS_SIZE),rngValMM(0,CHUNGUS_SIZE),tries+1);
	}
}

void worldgenTestpattern(worldgen *wgen){
	u8 b=1;
	for(int x=0;x<CHUNK_SIZE;x+=2){
		for(int y=0;y<CHUNK_SIZE;y+=2){
			for(int z=0;z<CHUNK_SIZE;z+=2){
				chungusFill(wgen->clay,x*CHUNK_SIZE,y*CHUNK_SIZE,z*CHUNK_SIZE,b);
				if(++b > 21){b=1;}
			}
		}
	}
}

void worldgenGenerate(worldgen *wgen){
	//static unsigned int averageTicks=0;
	static uint averageRuns=0;

	int oldSeed = getRNGSeed();
	//unsigned int startTicks = getTicks();
	uint seed = (optionWorldSeed | (optionWorldSeed << 16));
	seed ^= (wgen->gx&0xFF00) | (wgen->gx >> 8) | ((wgen->gx&0xFF00)<<8) | ((wgen->gx&0xFF00)<<16);
	seed ^= (wgen->gy&0xFF00) | (wgen->gy >> 8) | ((wgen->gy&0xFF00)<<8) | ((wgen->gy&0xFF00)<<16);
	seed ^= (wgen->gz&0xFF00) | (wgen->gz >> 8) | ((wgen->gz&0xFF00)<<8) | ((wgen->gz&0xFF00)<<16);

	seedRNG(seed);
	if(wgen->geoworld > 188){
		wgen->geoIslandChance = false;
		wgen->geoIslands      = true;
	}else if(wgen->geoworld > 156){
		wgen->geoIslandChance = true;
		wgen->geoIslands      = false;
	}else{
		wgen->geoIslandChance = false;
		wgen->geoIslands      = false;
	}
	switch(wgen->layer){
		default:
			worldgenTestpattern(wgen);
		break;

		case 15:
		case 14:
			worldgenCluster(wgen,CHUNGUS_SIZE/52,CHUNGUS_SIZE/52,3,9);
		break;
		case 13:
			worldgenLandmass(wgen,wgen->layer - 13);
		break;

		case 12:
		case 11:
		case 10:
		case 9:
			worldgenLabyrinth(wgen,wgen->layer - 8);
		break;

		case 8:
			worldgenCluster(wgen,CHUNGUS_SIZE/44,CHUNGUS_SIZE/44,4,12);
		break;

		case 7:
			worldgenCluster(wgen,CHUNGUS_SIZE/38,CHUNGUS_SIZE/38,7,22);
		break;

		case 6:
			worldgenCluster(wgen,CHUNGUS_SIZE/32,CHUNGUS_SIZE/32,6,22);
		break;

		case 5:
			worldgenCluster(wgen,CHUNGUS_SIZE/22,CHUNGUS_SIZE/28,4,16);
		break;

		case 4:
			worldgenCluster(wgen,CHUNGUS_SIZE/12,CHUNGUS_SIZE/22,4,8);
		break;

		case 3:
			worldgenCluster(wgen,CHUNGUS_SIZE/8,CHUNGUS_SIZE/32,1,3);
		break;

		case 2:
			worldgenCluster(wgen,CHUNGUS_SIZE/14,CHUNGUS_SIZE/24,2,5);
		break;

		case 1:
			worldgenCluster(wgen,CHUNGUS_SIZE/18,CHUNGUS_SIZE/22,5,12);
		break;

		case 0:
		case 127:
			return;
	}
	seedRNG(oldSeed);
	//averageTicks += getTicks()-startTicks;
	averageRuns++;
	//printf("Worldgen took %3ums (Avg.: %3ums   Total: %4ums) VC:%i ISM:%i ICM:%i\n",getTicks()-startTicks,averageTicks/averageRuns,averageTicks,vegetationConcentration,islandSizeModifier,islandCountModifier);
	//clay->debugInfo();
}
