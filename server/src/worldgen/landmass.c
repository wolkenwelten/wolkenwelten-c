#include "landmass.h"

#include "island.h"
#include "../voxel/bigchungus.h"
#include "../misc/noise.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void worldgenLandmass(worldgen *wgen, int layer){
	(void)layer;
	static unsigned char heightmap[256][256];
	
	//generateNoise(wgen->gx ^ wgen->gy ^ wgen->gz ^ 0xA39C13F1, heightmap);
	//fprintf(stderr,"heightModifier[%i][%i] = %i\n",wgen->gx,wgen->gz,wgen->heightModifier);
	generateNoiseZoomed(wgen->gx ^ wgen->gy ^ wgen->gz ^ 0xA39C13F1, heightmap,wgen->gx >> 8,wgen->gz >> 8,world.heightModifier);
	for(int x=0;x<CHUNGUS_SIZE;x++){
		for(int z=0;z<CHUNGUS_SIZE;z++){
			int h = heightmap[x][z]/4;
			if(h < 56){
				chungusBox (wgen->clay,x,CHUNGUS_SIZE/2,z,1,h/2,1,1);
				chungusBox (wgen->clay,x,(CHUNGUS_SIZE/2)-h/2,z,1,h/2,1,1);
			}else{
				chungusBox(wgen->clay,x,CHUNGUS_SIZE/2,z,1,h/2,1,3);
				chungusBox(wgen->clay,x,(CHUNGUS_SIZE/2)-h/2,z,1,h/2,1,3);
			}
			
		}
	}
	wgen->minX = 0;
	wgen->minY = 0;
	wgen->minZ = 0;
	
	wgen->maxX = CHUNGUS_SIZE;
	wgen->maxY = CHUNGUS_SIZE;
	wgen->maxZ = CHUNGUS_SIZE;
	
	worldgenRemoveDirt(wgen);
}