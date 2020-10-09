#include "landmass.h"

#include "island.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/noise.h"

void worldgenLandmass(worldgen *wgen, int layer){
	(void)layer;
	static unsigned char heightmap[256][256];
	static unsigned char diffmap[256][256];

	generateNoiseZoomed(wgen->gx ^ wgen->gy ^ wgen->gz ^ 0xA39C13F1, heightmap,wgen->gx >> 8,wgen->gz >> 8,world.heightModifier);
	generateNoise(wgen->gx ^ wgen->gy ^ wgen->gz ^ 0x49D5AB08, diffmap);
	for(int x=0;x<CHUNGUS_SIZE;x++){
		for(int z=0;z<CHUNGUS_SIZE;z++){
			int h = (heightmap[x][z]/8) + (diffmap[(x+128)&0xFF][(z+128)&0xFF]/16);
			if(h < 96){
				chungusBoxIfEmpty(wgen->clay,x,CHUNGUS_SIZE/2 + h/2,z,1,4,1,1);
				chungusBoxIfEmpty(wgen->clay,x,(CHUNGUS_SIZE/2)-h/2,z,1,h,1,3);
			}else{
				chungusBoxIfEmpty(wgen->clay,x,(CHUNGUS_SIZE/2)-h/2,z,1,h,1,3);
			}
			int r = rngValM(1024);
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
				case 14: {
					u8 b;
					switch(rngValM(128)){
						case 0:
							b = 18;
							break;
						case 1:
							b = 4;
							break;
						default:
							b = 3;
							break;
					}
					for(int sx = -3; sx < 4; sx++){
						for(int sz = -3; sz < 4; sz++){
							int y,d = 6 - (abs(sx) + abs(sz));
							d = d * d;
							switch(rngValM(256)){
								case 0:
									y = rngValMM(d,d*2);
									break;
								case 1:
								case 2:
								case 3:
								case 4:
								case 5:
								case 6:
								case 7:
								case 8:
									y = rngValMM(d/2,d);
									break;
								default:
									y = rngValMM(d/4,d/2);
									break;
							}

							chungusBox(wgen->clay, x+sx, CHUNGUS_SIZE/2 - (h/2) - y, z+sz,1, y*2,1, b);
						}
					}
					break; }
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
