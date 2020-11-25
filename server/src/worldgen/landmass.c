#include "landmass.h"

#include "island.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/noise.h"

#include <math.h>

static void genStalagtit(const worldgen *wgen, int x, int y, int z){
	u8 b = 3;
	switch(rngValM(78)){
	case 0:
		b = 18;
		break;
	case 1:
		b = 4;
		break;
	}
	for(int sx = -3; sx < 4; sx++){
		for(int sz = -3; sz < 4; sz++){
			int h,d = 6 - (abs(sx) + abs(sz));
			d = d * d;
			switch(rngValM(256)){
			case 0:
				h = rngValMM(d,d*2+1);
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				h = rngValMM(d/2,d+1);
				break;
			default:
				h = rngValMM(d/4,d/2+1);
				break;
			}
			chungusBox(wgen->clay, x+sx, y - h, z+sz,1, h*2,1, b);
		}
	}
}

static void genHole(const worldgen *wgen, int x, int z, int r, u8 heightmap[256][256]){
	(void)heightmap[0][0];
	int rr = r*r;
	int rh = (r/2)*(r/2);
	for(int sx = -r; sx <= r; sx++){
		for(int sz = -r; sz <= r; sz++){
			int dd = sx*sx + sz*sz;
			if(dd < rh){
				chungusBox(wgen->clay,x-sx,0,z-sz,1,CHUNGUS_SIZE,1,0);
			}else if(dd < rr){
				//int h = heightmap[(x+sx)&0xFF][(z+sz)&0xFF] / 4;
				int y = CHUNGUS_SIZE/2-(dd-rh)/4;
				chungusBox(wgen->clay,x-sx,y,z-sz,1,-y,1,0);
			}
		}
	}

}

void worldgenLandmass(worldgen *wgen, int layer){
	(void)layer;
	static u8 heightmap[256][256];
	static u8 diffmap[256][256];

	generateNoiseZoomed(wgen->gx ^ wgen->gy ^ wgen->gz ^ 0xA39C13F1, heightmap,wgen->gx >> 8,wgen->gz >> 8,world.heightModifier);
	generateNoise(wgen->gx ^ wgen->gy ^ wgen->gz ^ 0x49D5AB08, diffmap);
	for(int x=0;x<256;x++){
		for(int z=0;z<256;z++){
			heightmap[x][z] = heightmap[x][z]/2 + (diffmap[(x+128)&0xFF][(z+128)&0xFF]/4);
		}
	}

	for(int x=0;x<CHUNGUS_SIZE;x++){
		for(int z=0;z<CHUNGUS_SIZE;z++){
			int h = heightmap[x][z]/4;
			if(h < 96){
				chungusBoxIfEmpty(wgen->clay,x,CHUNGUS_SIZE/2 + h/2,z,1,4,1,1);
				chungusBoxIfEmpty(wgen->clay,x,(CHUNGUS_SIZE/2)-h/2,z,1,h,1,3);
			}else{
				chungusBoxIfEmpty(wgen->clay,x,(CHUNGUS_SIZE/2)-h/2,z,1,h,1,3);
			}
			int r = rngValM(4096);
			switch(r){
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				chungusBoxSphere(wgen->clay, x, CHUNGUS_SIZE/2 + (rngValM(h)-h/2), z, rngValMM(2,4), 4);
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				chungusBoxSphere(wgen->clay, x, CHUNGUS_SIZE/2 + (rngValM(h)-h/2), z, rngValMM(2,4), 13);
				break;
			case 10:
			case 11:
				chungusBoxSphere(wgen->clay, x, CHUNGUS_SIZE/2 + (rngValM(h)-h/2), z, rngValMM(1,3), 18);
				break;
			case 12:
				chungusBoxSphere(wgen->clay, x, CHUNGUS_SIZE/2 + (rngValM(h-12)-(h/2)), z, rngValMM(6,8), 4);
				break;
			case 13:
			case 14:
				genStalagtit(wgen,x,CHUNGUS_SIZE/2 - h/2,z);
				break;
			case 15:
				if(x < 32){continue;}
				if(z < 32){continue;}
				if(rngValM(4)!=0){continue;}
				genHole(wgen,x-16,z-16, rngValMM(4,16),heightmap);
				break;
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
