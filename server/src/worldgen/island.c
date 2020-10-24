#include "island.h"

#include "geoworld.h"
#include "vegetation.h"
#include "../game/animal.h"
#include "../voxel/chunk.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <stdlib.h>

void worldgenRock(worldgen *wgen,int x,int y,int z,int w,int h,int d){
	int iterations = 12;
	chungus *clay = wgen->clay;

	if((w < 8) || (h < 8) || (d < 8)){
		if((w <= 4) && (h <= 4) && (d <= 4)){
			int r = rngValM(64);
			switch(r){
				case 0:
				case 1:
				case 2:
				case 3:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,I_Hematite_Ore);
				break;

				case 4:
				case 5:
				case 6:
				case 7:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,I_Coal);
				break;

				case 8:
				case 9:
				case 10:
				case 11:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,I_Dirt);
				break;

				default:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,3);
				break;
			}
		}else{
			chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,3);
		}
		chungusBoxF(clay,x-w,y+h-2,z-d,w*2,2,d*2,1);
		if(x-w < wgen->minX){ wgen->minX = x-w; }
		if(y-h < wgen->minY){ wgen->minY = y-h; }
		if(z-d < wgen->minZ){ wgen->minZ = z-d; }
		if(x+w > wgen->maxX){ wgen->maxX = x+w; }
		if(y+h > wgen->maxY){ wgen->maxY = y+h; }
		if(z+d > wgen->maxZ){ wgen->maxZ = z+d; }
	}

	if((w < 2) || (h < 2) || (d < 2)){return;}
	for(int i=0;i<iterations;i++){
		int nx,ny,nz;
		int nw = w/2;
		int nh = h/2;
		int nd = d/2;
		if(wgen->iterChance > 0){
			if(rngValM(wgen->iterChance)==0){nw*=2;}
			if(rngValM(wgen->iterChance)==0){nh*=2;}
			if(rngValM(wgen->iterChance)==0){nd*=2;}
			if(rngValM(wgen->iterChance)==0){nw/=2;}
			if(rngValM(wgen->iterChance)==0){nh/=2;}
			if(rngValM(wgen->iterChance)==0){nd/=2;}
		}
		switch(rngValM(6)){
		case 0:
			nx = x - w;
			ny = rngValMM(y-h,y+h);
			nz = rngValMM(z-d,z+d);
		break;
		case 1:
			nx = x + w;
			ny = rngValMM(y-h,y+h);
			nz = rngValMM(z-d,z+d);
		break;
		case 2:
			nx = rngValMM(x-w,x+w);
			ny = y-h;
			nz = rngValMM(z-d,z+d);
		break;
		case 3:
			nx = rngValMM(x-w,x+w);
			ny = y+h;
			nz = rngValMM(z-d,z+d);
		break;
		case 4:
			nx = rngValMM(x-w,x+w);
			ny = rngValMM(y-h,y+h);
			nz = z - d;
		break;
		default:
		case 5:
			nx = rngValMM(x-w,x+w);
			ny = rngValMM(y-h,y+h);
			nz = z + d;
		break;
		}
		worldgenRock(wgen,nx,ny,nz,nw,nh,nd);
	}
}

void worldgenRemoveDirt(worldgen *wgen){
	chungus *clay = wgen->clay;
	int bigTreeChance  = 0;
	int treeChance     = 0;
	int deadTreeChance = 0;
	int shrubChance    = 0;
	int stoneChance    = 0;
	int dirtChance     = 0;
	int monolithChance = 0;
	int animalChance   = 0;
	int treeType = rngValM(2);
	bool hasSpecial = false;
	if(wgen->minX < 0){wgen->minX = 0;}
	if(wgen->minY < 0){wgen->minY = 0;}
	if(wgen->minZ < 0){wgen->minZ = 0;}
	if(wgen->maxX >= CHUNGUS_SIZE){wgen->maxX = CHUNGUS_SIZE-1;}
	if(wgen->maxY >= CHUNGUS_SIZE){wgen->maxY = CHUNGUS_SIZE-1;}
	if(wgen->maxZ >= CHUNGUS_SIZE){wgen->maxZ = CHUNGUS_SIZE-1;}

	switch(wgen->vegetationChance){
		case 7:
			bigTreeChance  =   148;
			treeChance     =    28;
			shrubChance    =    20;
			animalChance   =   384;
		break;
		case 6:
			bigTreeChance  =   192;
			treeChance     =    32;
			shrubChance    =    24;
			animalChance   =   512;
		break;
		default:
		case 5:
			bigTreeChance  =   256;
			treeChance     =    48;
			shrubChance    =    48;
			animalChance   =   768;
		break;
		case 4:
			bigTreeChance  =   512;
			treeChance     =    96;
			shrubChance    =    64;
			animalChance   =  1024;
		break;
		case 3:
			treeChance     =   768;
			shrubChance    =    96;
			dirtChance     =    32;
			stoneChance    =   256;
			animalChance   =  2048;
		break;
		case 2:
			treeChance     =  1024;
			shrubChance    =    64;
			dirtChance     =    12;
			stoneChance    =   128;
			animalChance   =  4096;
		break;
		case 1:
			shrubChance    =   256;
			dirtChance     =     4;
			stoneChance    =    64;
			treeChance     =  4096;
			deadTreeChance =  4096;
			animalChance   =  8192;
		break;
		case 0:
			shrubChance    =  1024;
			dirtChance     =     2;
			stoneChance    =    32;
			monolithChance = 32000;
			treeChance     =  8192*2;
			deadTreeChance =  8192;
			animalChance   =  8192*2;
		break;
	}
	(void)animalChance;

	for(int cz=wgen->minZ;cz<=wgen->maxZ;cz++){
		int z = cz&0xF;
		for(int cx=wgen->minX;cx<=wgen->maxX;cx++){
			int x = cx&0xF;
			int airBlocks = 8;
			u8 lastBlock = 0;
			chunk *chnk = NULL;
			for(int cy=wgen->maxY;cy>=wgen->minY;cy--){
				if((chnk == NULL) || ((cy&0xF)==0xF)){
					chnk = clay->chunks[cx>>4][cy>>4][cz>>4];
				}
				if(chnk == NULL){
					lastBlock = 0;
					airBlocks += cy - (cy&(~0xF));
					cy = cy & (~0xF);
					continue;
				}
				u8 b = chnk->data[x][cy&0xF][z];
				switch(b){
					default:
						airBlocks = 0;
					break;
					case 0:
					case 6:
					case 11:
						airBlocks++;
					break;

					case 1:
						if(!hasSpecial){
							if(monolithChance && (rngValM(monolithChance)==0)){
								worldgenMonolith(wgen,cx,cy,cz);
								lastBlock = 9;
								airBlocks = 0;
								hasSpecial = true;
								continue;
							}
						}

						if(bigTreeChance && (airBlocks > 32) && (rngValM(bigTreeChance)==0)){
							if(treeType==0){
								worldgenBigSpruce(wgen,cx,cy,cz);
							}else if(treeType == 1){
								worldgenBigOak(wgen,cx,cy,cz);
							}else{
								worldgenBigDeadTree(wgen,cx,cy,cz);
							}
							lastBlock = 5;
							airBlocks = 0;
							continue;
						}
						if(treeChance && (airBlocks > 16) && (rngValM(treeChance)==0)){
							if(treeType==0){
								worldgenSpruce(wgen,cx,cy,cz);
							}else if(treeType == 1){
								worldgenOak(wgen,cx,cy,cz);
							}
							lastBlock = 5;
							airBlocks = 0;
							continue;
						}
						if(animalChance && (airBlocks > 4) && (rngValM(animalChance)==0)){
							animalNew(vecNew(wgen->gx+cx,wgen->gy+cy+1.f,wgen->gz+cz),1,0);
							animalNew(vecNew(wgen->gx+cx,wgen->gy+cy+1.f,wgen->gz+cz),1,1);
						}
						if(deadTreeChance && (airBlocks > 16) && (rngValM(deadTreeChance)==0)){
							worldgenDeadTree(wgen,cx,cy,cz);
							lastBlock = 5;
							airBlocks = 0;
							continue;
						}
						if(airBlocks > 8){
							if(shrubChance && (rngValM(shrubChance)==12)){
								worldgenShrub(wgen,cx,cy,cz);
								lastBlock = b;
								airBlocks = 0;
								continue;
							}
							if((dirtChance) && (rngValM(dirtChance)==1)){
								lastBlock = b;
								airBlocks = 0;
								continue;
							}
							if(stoneChance && (rngValM(stoneChance)==1)){
								chnk->data[x][cy&0xF][z] = 3;
								chungusSetB(clay,cx,cy+1,cz,3);
								continue;
							}
							lastBlock = 1;
							airBlocks = 0;
							chnk->data[x][cy&0xF][z] = 2;
							continue;
						}
						if((lastBlock == 1) || (lastBlock == 2)){
							chnk->data[x][cy&0xF][z] = 1;
						}else{
							lastBlock = chnk->data[x][cy&0xF][z] = 3;
							continue;
						}
						airBlocks = 0;
					break;
				}
				lastBlock = b;
			}
		}
	}
}

void worldgenDirtIsland(worldgen *wgen, int x,int y,int z,int size){
	int w,h,d;
	w = h = d = size;
	switch(rngValM(48)){
		case 1:
			w/=2;
		break;
		case 2:
		case 3:
			h/=2;
		break;
		case 4:
			d/=2;
		break;
	}
	worldgenRock(wgen,x,y,z,w,h,d);
	worldgenRemoveDirt(wgen);
}

void worldgenIsland(worldgen *wgen, int x,int y,int z,int size){
	wgen->minX = wgen->maxX = x;
	wgen->minY = wgen->maxY = y;
	wgen->minZ = wgen->maxZ = z;
	wgen->vegetationChance = wgen->vegetationConcentration/32;

	wgen->iterChance = rngValM(4)*8;
	if(wgen->geoIslands){
		worldgenGeoIsland(wgen,x,y,z,size);
	}else if(wgen->geoIslandChance && (rngValM(8)==0)){
		worldgenGeoIsland(wgen,x,y,z,MIN(6,size));
	}else{
		worldgenDirtIsland(wgen,x,y,z,size);
	}
}

void worldgenSCluster(worldgen *wgen, int x,int y,int z,int size,int csize){
	if(rngValM(4)==0){
		worldgenIsland(wgen,x,y,z,size);
	}
	for(int i=rngValMM(128,256);i>0;i--){
		int nx = x + rngValM(csize*4)-csize*2;
		int ny = y + rngValM(csize*4)-csize*2;
		int nz = z + rngValM(csize*4)-csize*2;
		worldgenIsland(wgen,nx,ny,nz,size/4);
	}
}

void worldgenCluster(worldgen *wgen, int size, int iSize, int iMin,int iMax){
	if(wgen->islandCountModifier < 128){
		iMin -= ((float)iMin*((128.f-wgen->islandCountModifier)/128.f));
		iMax -= ((float)iMax*((128.f-wgen->islandCountModifier)/128.f));
	}else{
		iMin += ((float)iMin*((wgen->islandCountModifier-128.f)/128.f));
		iMax += ((float)iMax*((wgen->islandCountModifier-128.f)/128.f));
	}
	if(wgen->islandSizeModifier < 128){
		size  -= ((float) size*((128.f-wgen->islandSizeModifier)/128.f));
		iSize -= ((float)iSize*((128.f-wgen->islandSizeModifier)/128.f));
	}else{
		size  += ((float) size*((wgen->islandSizeModifier-128.f)/128.f));
		iSize += ((float)iSize*((wgen->islandSizeModifier-128.f)/128.f));
	}

	int xoff = rngValM(CHUNGUS_SIZE-(size*4))-(CHUNGUS_SIZE-(size*4))/2;
	int yoff = rngValM(CHUNGUS_SIZE-(size*4))-(CHUNGUS_SIZE-(size*4))/2;
	int zoff = rngValM(CHUNGUS_SIZE-(size*4))-(CHUNGUS_SIZE-(size*4))/2;
	if(rngValM(6)==1){
		worldgenIsland(wgen,CHUNGUS_SIZE/2+xoff,CHUNGUS_SIZE/2+yoff,CHUNGUS_SIZE/2+zoff,size);
	}else{
		iMin*=2;
	}
	if(wgen->geoIslands){
		iMin *= 12;
		iMax *= 12;
		iSize /= 2;
	}
	for(int i=rngValMM(iMin,iMax);i>0;i--){
		int nx = rngValM(CHUNGUS_SIZE-CHUNGUS_SIZE/8)+CHUNGUS_SIZE/16;
		int ny = rngValM(CHUNGUS_SIZE-CHUNGUS_SIZE/8)+CHUNGUS_SIZE/16;
		int nz = rngValM(CHUNGUS_SIZE-CHUNGUS_SIZE/8)+CHUNGUS_SIZE/16;
		worldgenIsland(wgen,nx,ny,nz,iSize);
	}
	worldgenFindSpawn(wgen,CHUNGUS_SIZE/2+xoff,CHUNGUS_SIZE/2+zoff,0);
}
