#include "worldgen.h"

#include "../main.h"
#include "../misc/options.h"
#include "../voxel/chunk.h"
#include "../voxel/chungus.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

worldgen worldgenList[4];
int worldgenCount = 0;
worldgen *worldgenFirstFree = NULL;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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

	wgen->vegetationConcentration = world.vegetationConcentration[gx][gz];
	wgen->islandSizeModifier      = world.islandSizeModifier     [gx][gz];
	wgen->islandCountModifier     = world.islandCountModifier    [gx][gz];
	wgen->geoworld                = world.geoworldMap            [gx][gz] + ((gy & 0xF0) << 2);

	return wgen;
}

void worldgenFree(worldgen *wgen){
	wgen->nextFree = worldgenFirstFree;
	worldgenFirstFree = wgen;
}

void worldgenRock(worldgen *wgen,int x,int y,int z,int w,int h,int d){
	int iterations = 12;
	chungus *clay = wgen->clay;

	if((w < 8) || (h < 8) || (d < 8)){
		if((w <= 4) && (h <= 4) && (d <= 4)){
			int r = rngValM(16);
			switch(r){
				case 0:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,13);
				break;
				case 1:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,4);
				break;
				case 2:
					chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,1);
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

void worldgenShrub(worldgen *wgen,int x,int y,int z){
	chungus *clay = wgen->clay;
	int leaveBlock = 11;
	if(rngValM(2) == 0){leaveBlock = 6;}
	chungusSetB(clay,x,y,z,8);

	chungusSetB(clay,x,y+1,z,leaveBlock);
	if(rngValM(2) == 0){
		chungusSetB(clay,x,y+2,z,leaveBlock);
	}
}

void worldgenRoots(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size = rngValMM(4,12);
	for(int cy = 0;cy > -size;--cy){
		uint8_t b = chungusGetB(clay,x,y+cy,z);
		switch(b){
			case 7:
			case 6:
			case 0:
				chungusSetB(clay,x,y+cy,z,7);
			break;

			case 1:
			case 2:
			case 8:
				chungusSetB(clay,x,y+cy,z,8);
			break;

			default:
				return;
		}
		b = chungusGetB(clay,x-1,y+cy,z);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			chungusSetB(clay,x-1,y+cy,z,8);
		}
		b = chungusGetB(clay,x+1,y+cy,z);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			chungusSetB(clay,x+1,y+cy,z,8);
		}
		b = chungusGetB(clay,x,y+cy,z-1);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			chungusSetB(clay,x,y+cy,z-1,8);
		}
		b = chungusGetB(clay,x,y+cy,z+1);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			chungusSetB(clay,x,y+cy,z+1,8);
		}
	}
}

void worldgenBigRoots(worldgen *wgen, int x,int y,int z){
	for(int cx = 0;cx < 2;cx++){
		for(int cz = 0;cz < 2;cz++){
			worldgenRoots(wgen,cx+x,y,cz+z);
		}
	}
}

void worldgenDeadTree(worldgen *wgen, int x,int y,int z){
	int size       = rngValMM(12,16);
	for(int cy = 0;cy < size;cy++){
		chungusSetB(wgen->clay,x,cy+y,z,5);
	}
	worldgenRoots(wgen,x,y-1,z);
}

void worldgenBigDeadTree(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(20,34);
	for(int cy = -5;cy < size;cy++){
		if(cy < -2){
			chungusSetB(clay,x,cy+y,z,8);
			chungusSetB(clay,x+1,cy+y,z,8);
			chungusSetB(clay,x,cy+y,z+1,8);
			chungusSetB(clay,x+1,cy+y,z+1,8);
		}else{
			chungusSetB(clay,x,cy+y,z,5);
			chungusSetB(clay,x+1,cy+y,z,5);
			chungusSetB(clay,x,cy+y,z+1,5);
			chungusSetB(clay,x+1,cy+y,z+1,5);
		}
	}
	worldgenBigRoots(wgen,x,y-5,z);
}

void worldgenSpruce(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(12,16);
	int sparseness = rngValMM(2,6);
	int lsize      = 2;

	for(int cy = 0;cy < size;cy++){
		if(cy > size/2){
			lsize = 1;
		}else{
			lsize = 2;
		}
		if(cy >= 4){
			for(int cz = -lsize;cz<=lsize;cz++){
				for(int cx = -lsize;cx<=lsize;cx++){
					if((rngValM(sparseness)) == 0){continue;}
					chungusSetB(clay,cx+x,cy+y,cz+z,6);
				}
			}
		}
		chungusSetB(clay,x,cy+y,z,5);
	}
	chungusSetB(clay,x,size+y,z,6);
	chungusSetB(clay,x,size+y+1,z,6);
	worldgenRoots(wgen, x,y-1,z);
}

void worldgenBigSpruce(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(20,34);
	int sparseness = rngValMM(3,5);
	int lsize      = 8;

	for(int cy = -5;cy < size;cy++){
		lsize = (size-cy)/4;
		if(lsize < 2){lsize=1;}
		if(cy >= 8){
			for(int cz = -lsize;cz<=lsize+1;cz++){
				for(int cx = -lsize;cx<=lsize+1;cx++){
					if(rngValM(sparseness) == 0){continue;}
					chungusSetB(clay,cx+x,cy+y,cz+z,6);
				}
			}
		}
		if(cy < -2){
			chungusSetB(clay,x,cy+y,z,8);
			chungusSetB(clay,x+1,cy+y,z,8);
			chungusSetB(clay,x,cy+y,z+1,8);
			chungusSetB(clay,x+1,cy+y,z+1,8);
		}else{
			chungusSetB(clay,x,cy+y,z,5);
			chungusSetB(clay,x+1,cy+y,z,5);
			chungusSetB(clay,x,cy+y,z+1,5);
			chungusSetB(clay,x+1,cy+y,z+1,5);
		}
	}
	for(int cx=0;cx<2;cx++){
		for(int cy=0;cy<2;cy++){
			for(int cz=0;cz<2;cz++){
				chungusSetB(clay,x+cx,size+y+cy,z+cz,6);
			}
		}
	}
	worldgenBigRoots(wgen,x,y-5,z);
}

void worldgenOak(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(8,12);
	int sparseness = rngValMM(2,3);
	int lsize      = 3;
	int leafes     = 11;
	int log        = 10;
	if(rngValM(16) == 0){leafes = 19;} // We Sakura now
	if(rngValM(16) == 0){log    = 20;} // We Birch now

	for(int cy = 0;cy < size;cy++){
		if(cy == size-1){lsize=2;}
		else if(cy == 5){lsize=2;}
		else {lsize = 3;}
		if(cy >= 5){
			for(int cz = -lsize;cz<=lsize;cz++){
				for(int cx = -lsize;cx<=lsize;cx++){
					if((cx == 0) && (cz == 0)){
						chungusSetB(clay,cx+x,cy+y,cz+z,leafes);
						continue;
					}
					if((cx == -lsize  ) && (cz == -lsize  )){continue;}
					if((cx == -lsize  ) && (cz ==  lsize))  {continue;}
					if((cx ==  lsize) && (cz == -lsize  ))  {continue;}
					if((cx ==  lsize) && (cz ==  lsize))    {continue;}
					if((rngValM(sparseness)) == 0)          {continue;}
					chungusSetB(clay,cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < size-2){
			chungusSetB(clay,x,cy+y,z,log);
		}
	}
	worldgenRoots(wgen,x,y-1,z);
}

void worldgenBigOak(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(18,24);
	int sparseness = rngValMM(2,4);
	int lsize      = 8;

	for(int cy = -5;cy < size;cy++){
		lsize = (cy-9);
		if((size - cy) < lsize){
			lsize = size-cy;
		}
		if(cy >= 9){
			for(int cz = -lsize;cz<=lsize+1;cz++){
				for(int cx = -lsize;cx<=lsize+1;cx++){
					if((cx == -lsize  ) && (cz == -lsize  )){continue;}
					if((cx == -lsize  ) && (cz ==  lsize+1)){continue;}
					if((cx ==  lsize+1) && (cz == -lsize  )){continue;}
					if((cx ==  lsize+1) && (cz ==  lsize+1)){continue;}
					if(rngValM(sparseness) == 0)            {continue;}
					chungusSetB(clay,cx+x,cy+y,cz+z,11);
				}
			}
		}
		if(cy < -2){
			chungusSetB(clay,x,cy+y,z,8);
			chungusSetB(clay,x+1,cy+y,z,8);
			chungusSetB(clay,x,cy+y,z+1,8);
			chungusSetB(clay,x+1,cy+y,z+1,8);
		}else if(cy < size-2){
			chungusSetB(clay,x,cy+y,z,10);
			chungusSetB(clay,x+1,cy+y,z,10);
			chungusSetB(clay,x,cy+y,z+1,10);
			chungusSetB(clay,x+1,cy+y,z+1,10);
		}
	}
	worldgenBigRoots(wgen,x,y-5,z);
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

void worldgenSphere(worldgen *wgen, int x,int y,int z,int size,int b){
	float rsq      = (size*size);
	float crystalr = rsq / 2.f;
	if(crystalr > 16.f){crystalr = 0.f;}

	for(int cy=-size;cy<=size;cy++){
		for(int cx = -size;cx <= size;cx++){
			for(int cz = -size;cz <= size;cz++){
				const float d = (cx*cx)+(cz*cz)+(cy*cy);
				if(d < crystalr){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,18);
				}else if(d < rsq){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
				}
			}
		}
	}
}

void worldgenRoundPrism(worldgen *wgen, int x,int y,int z,int size,int b){
	for(int cy=-size;cy<=size;cy++){
		int r     = (size-abs(cy))/2;
		float rsq = (r*r)*0.8f;
		float crystalr = rsq / 2.f;
		if(crystalr < 16.f){crystalr = 0.f;}
		for(int cx = -r;cx <= r;cx++){
			for(int cz = -r;cz <= r;cz++){
				const float d = (cx*cx)+(cz*cz);
				if(d < crystalr){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,18);
				} else if(d < rsq){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
				}
			}
		}
	}
}

void worldgenPrism(worldgen *wgen, int x,int y,int z,int size,int b){
	for(int cy=-size;cy<=size;cy++){
		int r  = (size-abs(cy))/2;
		int cr = r / 2;
		if(r < 2){cr = 0;}
		for(int cx = -r;cx <= r;cx++){
			for(int cz = -r;cz <= r;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
			}
		}
		if(cr == 0){continue;}
		for(int cx = -cr;cx <= cr;cx++){
			for(int cz = -cr;cz <= cr;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,18);
			}
		}
	}
}

void worldgenPyramid(worldgen *wgen, int x,int y,int z,int size,int b){
	for(int cy=-size;cy<=size;cy++){
		int r  = size-abs(cy);
		int cr = r / 2;
		if(r < 2){cr = 0;}
		for(int cx = -r;cx <= r;cx++){
			for(int cz = -r;cz <= r;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
			}
		}
		if(cr == 0){continue;}
		for(int cx = -cr;cx <= cr;cx++){
			for(int cz = -cr;cz <= cr;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,18);
			}
		}
	}
}

void worldgenCube(worldgen *wgen, int x, int y, int z, int size, int b){
	chungusBox(wgen->clay,x,y,z,size,size,size,b);
	if(size > 4){
		chungusBox(wgen->clay,x+size/4,y+size/4,z+size/4,size/2,size/2,size/2,18);
	}
}

void worldgenRemoveDirt(worldgen *wgen){
	chungus *clay = wgen->clay;
	int bigTreeChance  = 0;
	int treeChance     = 0;
	int shrubChance    = 0;
	int stoneChance    = 0;
	int dirtChance     = 0;
	int monolithChance = 0;
	//int obeliskChance  = 0;
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
			bigTreeChance = 148;
			treeChance    = 28;
			shrubChance   = 20;
			//obeliskChance = 32000;
		break;
		case 6:
			bigTreeChance = 192;
			treeChance    = 32;
			shrubChance   = 24;
			//obeliskChance = 32000;
		break;
		default:
		case 5:
			bigTreeChance = 256;
			treeChance    = 48;
			shrubChance   = 48;
		break;
		case 4:
			bigTreeChance = 512;
			treeChance    = 96;
			shrubChance   = 64;
		break;
		case 3:
			treeChance    = 256;
			shrubChance   = 96;
			stoneChance   = 256;
		break;
		case 2:
			shrubChance   = 64;
			dirtChance    = 32;
			stoneChance   = 128;
		break;
		case 1:
			shrubChance    = 256;
			dirtChance     = 3;
			stoneChance    = 64;
			treeChance     = 1024;
			treeType       = 2;
		break;
		case 0:
			shrubChance    = 1024;
			dirtChance     = 2;
			stoneChance    = 32;
			monolithChance = 32000;
			treeChance     = 2048;
			treeType       = 2;
		break;
	}

	for(int cz=wgen->minZ;cz<wgen->maxZ;cz++){
		int z = cz&0xF;
		for(int cx=wgen->minX;cx<wgen->maxX;cx++){
			int x = cx&0xF;
			int airBlocks=8;
			uint8_t lastBlock=0;
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
				uint8_t b = chnk->data[x][cy&0xF][z];
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
							/*
							if(obeliskChance && (rngVal(obeliskChance)==12)){
								generateObelisk(cx,cy,cz);
								lastBlock = 12;
								airBlocks = 0;
								hasSpecial = true;
								continue;
							}*/
							if(monolithChance && (rngValM(monolithChance)==12)){
								worldgenMonolith(wgen,cx,cy,cz);
								lastBlock = 9;
								airBlocks = 0;
								hasSpecial = true;
								continue;
							}
						}

						if(bigTreeChance && (airBlocks > 32) && (rngValM(bigTreeChance)==12)){
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
						if(treeChance && (airBlocks > 16) && (rngValM(treeChance)==12)){
							if(treeType==0){
								worldgenSpruce(wgen,cx,cy,cz);
							}else if(treeType == 1){
								worldgenOak(wgen,cx,cy,cz);
							}else{
								worldgenDeadTree(wgen,cx,cy,cz);
							}
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

void worldgenGeoIsland(worldgen *wgen, int x,int y,int z,int size){
	int b=12;
	switch(rngValM(2)){
		case 0:
			b = 9;
		break;
		default:
		case 1:
			b = 12;
		break;
	}

	x &= 0xF0;
	y &= 0xF0;
	z &= 0xF0;
	if(x == 0){x += 0x10;}
	if(y == 0){y += 0x10;}
	if(z == 0){z += 0x10;}
	if(x == 0xF0){x -= 0x10;}
	if(y == 0xF0){y -= 0x10;}
	if(z == 0xF0){z -= 0x10;}

	switch(rngValM(96)){
		case 0:
			size *= 9;
		break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			size *= 3;
		break;
	}

	switch(rngValM(5)){
		default:
		case 0:
			worldgenCube(wgen,x,y,z,size,b);
		break;
		case 1:
			worldgenPrism(wgen,x,y,z,size,b);
		break;
		case 2:
			worldgenPyramid(wgen,x,y,z,size,b);
		break;
		case 3:
			worldgenRoundPrism(wgen,x,y,z,size,b);
		break;
		case 4:
			worldgenSphere(wgen,x,y,z,size,b);
		break;
	}
}

void worldgenIsland(worldgen *wgen, int x,int y,int z,int size){
	wgen->minX = wgen->maxX = x;
	wgen->minY = wgen->maxY = y;
	wgen->minZ = wgen->maxZ = z;
	wgen->vegetationChance = wgen->vegetationConcentration/32;

	wgen->iterChance = rngValM(4)*8;
	if(wgen->geoIslands){
		worldgenGeoIsland(wgen,x,y,z,size);
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

void worldgenFindSpawn(worldgen *wgen, int x,int z,int tries){
	if(tries > 8){
		wgen->clay->spawnx = wgen->clay->spawny = wgen->clay->spawnz = -1;
		return;
	}
	int y = 0;
	if(chungusGetHighestP(wgen->clay,x,&y,z)){
		wgen->clay->spawnx = x;
		wgen->clay->spawny = y;
		wgen->clay->spawnz = z;
	}else{
		worldgenFindSpawn(wgen,rngValMM(0,CHUNGUS_SIZE),rngValMM(0,CHUNGUS_SIZE),tries+1);
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
		size -= ((float)size*((128.f-wgen->islandSizeModifier)/128.f));
		iSize -= ((float)iSize*((128.f-wgen->islandSizeModifier)/128.f));
	}else{
		size += ((float)size*((wgen->islandSizeModifier-128.f)/128.f));
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

void worldgenLabyrinth(worldgen *wgen, int labLayer){
	chungus *clay = wgen->clay;
	unsigned char labMap[16][16][16];
	int b = 12;
	int pb = 14;
	int fb = 15;
	memset(labMap,0,sizeof(labMap));
	//if(geoworld > 128){ b = 12;}
	for(int cx = 15; cx >= 0; cx--){
		for(int cy = 15; cy >= 0; cy--){
			for(int cz = 15; cz >= 0; cz--){
				if(labLayer == 1){
					if(rngValM(5) != 0){continue;}
				}
				if(labLayer == 2){
					if(rngValM(3) != 0){continue;}
				}
				if(labLayer == 3){
					if(rngValM(2) == 0){continue;}
				}
				if(labLayer == 4){
					if(rngValM(4) == 0){continue;}
				}
				labMap[cx][cy][cz] = 1;
			}
		}
	}

	for(int i=0;i<4;i++){
		for(int cx = 15; cx >= 0; cx--){
			for(int cy = 15; cy >= 0; cy--){
				for(int cz = 15; cz >= 0; cz--){
					int n = 0;
					if((cx >  0) && labMap[cx-1][cy][cz]){++n;}
					if((cx < 15) && labMap[cx+1][cy][cz]){++n;}

					if((cy >  0) && labMap[cx][cy-1][cz]){n+=6;}
					if((cy < 15) && labMap[cx][cy+1][cz]){n+=6;}

					if((cz >  0) && labMap[cx][cy][cz-1]){++n;}
					if((cz < 15) && labMap[cx][cy][cz+1]){++n;}

					if((n > 5) || (n == 0)){ labMap[cx][cy][cz] = 0; }
				}
			}
		}
	}


	for(int cx = 15; cx >= 0; cx--){
		for(int cy = 15; cy >= 0; cy--){
			for(int cz = 15; cz >= 0; cz--){
				const int bx = (cx<<4)+3;
				const int by = (cy<<4)+6;
				const int bz = (cz<<4)+3;
				if(!labMap[cx][cy][cz]){continue;}

				chungusBox(clay,bx+1,by-1,bz+1, 8,1, 8,b);
				chungusBox(clay,bx  ,by  ,bz  ,10,1,10,fb);

				chungusBox(clay,bx  ,by+1,bz  ,10,1, 1,b);
				chungusBox(clay,bx  ,by+1,bz+9,10,1, 1,b);

				chungusBox(clay,bx  ,by+1,bz  , 1,1,10,b);
				chungusBox(clay,bx+9,by+1,bz  , 1,1,10,b);

				switch(rngValM(16)){
					default:
					case 0:
					break;

					case 1:
						chungusBox(clay,bx  ,by+2,bz  ,1,5,1,pb);
						chungusBox(clay,bx+9,by+2,bz  ,1,5,1,pb);
						chungusBox(clay,bx  ,by+2,bz+9,1,5,1,pb);
						chungusBox(clay,bx+9,by+2,bz+9,1,5,1,pb);

						chungusBox(clay,bx  ,by+7,bz  ,10,1,10,b);
						chungusBox(clay,bx+1,by+7,bz+1, 8,1, 8,0);
						chungusBox(clay,bx+1,by+8,bz+1, 8,1, 8,b);
					break;
				}

				if((cx < 15) && labMap[cx+1][cy][cz]){
					if(rngValM(4)==0){
						chungusBox(clay,bx+ 9,by+1,bz+1,8,2,8,0);
						chungusBox(clay,bx+10,by  ,bz+1,6,1,8,fb);
						chungusBox(clay,bx+10,by+1,bz,6,1,1,b);
						chungusBox(clay,bx+10,by+1,bz+9,6,1,1,b);
					}else{
						chungusBox(clay,bx+10,by  ,bz+3,6,1,4,fb);
						chungusBox(clay,bx+ 9,by+1,bz+4,8,2,2,0);
						chungusBox(clay,bx+10,by+1,bz+3,6,1,1,b);
						chungusBox(clay,bx+10,by+1,bz+6,6,1,1,b);
					}
				}
				if((cz < 15) && labMap[cx][cy][cz+1]){
					if(rngValM(4)==0){
						chungusBox(clay,bx+1,by+1,bz+ 9,8,2,8,0);
						chungusBox(clay,bx+1,by  ,bz+10,8,1,6,fb);
						chungusBox(clay,bx  ,by+1,bz+10,1,1,6,b);
						chungusBox(clay,bx+9,by+1,bz+10,1,1,6,b);
					}else{
						chungusBox(clay,bx+3,by  ,bz+10,4,1,6,fb);
						chungusBox(clay,bx+4,by+1,bz+ 9,2,2,8,0);
						chungusBox(clay,bx+3,by+1,bz+10,1,1,6,b);
						chungusBox(clay,bx+6,by+1,bz+10,1,1,6,b);
					}
				}

				if((cx > 0) && (cy < 15) && labMap[cx-1][cy+1][cz]){
					chungusSetB(clay,bx+4,by+1,bz+1,0);
					chungusSetB(clay,bx+4,by+1,bz+2,0);
					for(int i=0;i<16;i++){
						chungusSetB(clay,bx-i+4,by+i-1,bz+1,b);
						chungusSetB(clay,bx-i+4,by+i-1,bz+2,b);

						chungusSetB(clay,bx-i+4,by+i,bz,b);
						chungusSetB(clay,bx-i+4,by+i,bz+1,fb);
						chungusSetB(clay,bx-i+4,by+i,bz+2,fb);
						chungusSetB(clay,bx-i+4,by+i,bz+3,b);

						chungusSetB(clay,bx-i+4,by+i+1,bz,b);
						chungusSetB(clay,bx-i+4,by+i+1,bz+3,b);

						chungusSetB(clay,bx-i+4,by+i+1,bz+1,0);
						chungusSetB(clay,bx-i+4,by+i+1,bz+2,0);
						chungusSetB(clay,bx-i+4,by+i+2,bz+1,0);
						chungusSetB(clay,bx-i+4,by+i+2,bz+2,0);
						chungusSetB(clay,bx-i+4,by+i+3,bz+1,0);
						chungusSetB(clay,bx-i+4,by+i+3,bz+2,0);
						chungusSetB(clay,bx-i+4,by+i+4,bz+1,0);
						chungusSetB(clay,bx-i+4,by+i+4,bz+2,0);
					}
				}
				if((cx < 15) && (cy > 0) && labMap[cx+1][cy-1][cz]){
					for(int i=0;i<6;i++){
						chungusSetB(clay,bx+3+i,by-i+2,bz+1,0);
						chungusSetB(clay,bx+3+i,by-i+2,bz+2,0);
						chungusSetB(clay,bx+4+i,by-i+2,bz+1,0);
						chungusSetB(clay,bx+4+i,by-i+2,bz+2,0);
						chungusSetB(clay,bx+3+i,by-i+3,bz+1,0);
						chungusSetB(clay,bx+3+i,by-i+3,bz+2,0);
						chungusSetB(clay,bx+4+i,by-i+3,bz+1,0);
						chungusSetB(clay,bx+4+i,by-i+3,bz+2,0);
					}
				}

				if((cz > 0) && (cy < 15) && labMap[cx][cy+1][cz-1]){
					chungusSetB(clay,bx+1,by+1,bz+4,0);
					chungusSetB(clay,bx+2,by+1,bz+4,0);
					for(int i=0;i<16;i++){
						chungusSetB(clay,bx+1,by+i-1,bz+4-i,b);
						chungusSetB(clay,bx+2,by+i-1,bz+4-i,b);

						chungusSetB(clay,bx  ,by+i,bz+4-i,b);
						chungusSetB(clay,bx+1,by+i,bz+4-i,fb);
						chungusSetB(clay,bx+2,by+i,bz+4-i,fb);
						chungusSetB(clay,bx+3,by+i,bz+4-i,b);

						chungusSetB(clay,bx  ,by+i+1,bz+4-i,b);
						chungusSetB(clay,bx+3,by+i+1,bz+4-i,b);

						chungusSetB(clay,bx+1,by+i+1,bz+4-i,0);
						chungusSetB(clay,bx+2,by+i+1,bz+4-i,0);
						chungusSetB(clay,bx+1,by+i+2,bz+4-i,0);
						chungusSetB(clay,bx+2,by+i+2,bz+4-i,0);
						chungusSetB(clay,bx+1,by+i+3,bz+4-i,0);
						chungusSetB(clay,bx+2,by+i+3,bz+4-i,0);
						chungusSetB(clay,bx+1,by+i+4,bz+4-i,0);
						chungusSetB(clay,bx+2,by+i+4,bz+4-i,0);
					}
				}
				if((cz < 15) && (cy > 0) && labMap[cx][cy-1][cz+1]){
					for(int i=0;i<6;i++){
						chungusSetB(clay,bx+1,by-i+2,bz+3+i,0);
						chungusSetB(clay,bx+2,by-i+2,bz+3+i,0);
						chungusSetB(clay,bx+1,by-i+2,bz+4+i,0);
						chungusSetB(clay,bx+2,by-i+2,bz+4+i,0);
						chungusSetB(clay,bx+1,by-i+3,bz+3+i,0);
						chungusSetB(clay,bx+2,by-i+3,bz+3+i,0);
						chungusSetB(clay,bx+1,by-i+3,bz+4+i,0);
						chungusSetB(clay,bx+2,by-i+3,bz+4+i,0);
					}
				}

				if((cx > 0) && (cy > 0) && labMap[cx-1][cy-1][cz]){
					chungusSetB(clay,bx+6,by+1,bz+1,0);
					chungusSetB(clay,bx+6,by+1,bz+2,0);
					for(int i=0;i<16;i++){
						chungusSetB(clay,bx-i+6,by-i-1,bz+7,b);
						chungusSetB(clay,bx-i+6,by-i-1,bz+8,b);

						chungusSetB(clay,bx-i+6,by-i,bz+6,b);
						chungusSetB(clay,bx-i+6,by-i,bz+7,fb);
						chungusSetB(clay,bx-i+6,by-i,bz+8,fb);
						chungusSetB(clay,bx-i+6,by-i,bz+9,b);

						chungusSetB(clay,bx-i+6,by-i+1,bz+6,b);
						chungusSetB(clay,bx-i+6,by-i+1,bz+9,b);

						chungusSetB(clay,bx-i+6,by-i+1,bz+7,0);
						chungusSetB(clay,bx-i+6,by-i+1,bz+8,0);
						chungusSetB(clay,bx-i+6,by-i+2,bz+7,0);
						chungusSetB(clay,bx-i+6,by-i+2,bz+8,0);
						chungusSetB(clay,bx-i+6,by-i+3,bz+7,0);
						chungusSetB(clay,bx-i+6,by-i+3,bz+8,0);
						chungusSetB(clay,bx-i+6,by-i+4,bz+7,0);
						chungusSetB(clay,bx-i+6,by-i+4,bz+8,0);
					}
				}
				if((cx < 15) && (cy < 15) && labMap[cx+1][cy+1][cz]){
					for(int i=0;i<6;i++){
						chungusSetB(clay,bx+i+6,by+i+1,bz+7,0);
						chungusSetB(clay,bx+i+6,by+i+1,bz+8,0);
						chungusSetB(clay,bx+i+6,by+i+2,bz+7,0);
						chungusSetB(clay,bx+i+6,by+i+2,bz+8,0);
						chungusSetB(clay,bx+i+6,by+i+3,bz+7,0);
						chungusSetB(clay,bx+i+6,by+i+3,bz+8,0);
						chungusSetB(clay,bx+i+6,by+i+4,bz+7,0);
						chungusSetB(clay,bx+i+6,by+i+4,bz+8,0);
					}
				}
				if((cz > 0) && (cy > 0) && labMap[cx][cy-1][cz-1]){
					chungusSetB(clay,bx+1,by+1,bz+6,0);
					chungusSetB(clay,bx+2,by+1,bz+6,0);
					for(int i=0;i<16;i++){
						chungusSetB(clay,bx+7,by-i-1,bz-i+6,b);
						chungusSetB(clay,bx+8,by-i-1,bz-i+6,b);

						chungusSetB(clay,bx+6,by-i,bz-i+6,b);
						chungusSetB(clay,bx+7,by-i,bz-i+6,fb);
						chungusSetB(clay,bx+8,by-i,bz-i+6,fb);
						chungusSetB(clay,bx+9,by-i,bz-i+6,b);

						chungusSetB(clay,bx+6,by-i+1,bz-i+6,b);
						chungusSetB(clay,bx+9,by-i+1,bz-i+6,b);

						chungusSetB(clay,bx+7,by-i+1,bz-i+6,0);
						chungusSetB(clay,bx+8,by-i+1,bz-i+6,0);
						chungusSetB(clay,bx+7,by-i+2,bz-i+6,0);
						chungusSetB(clay,bx+8,by-i+2,bz-i+6,0);
						chungusSetB(clay,bx+7,by-i+3,bz-i+6,0);
						chungusSetB(clay,bx+8,by-i+3,bz-i+6,0);
						chungusSetB(clay,bx+7,by-i+4,bz-i+6,0);
						chungusSetB(clay,bx+8,by-i+4,bz-i+6,0);
					}
				}
				if((cz < 15) && (cy < 15) && labMap[cx][cy+1][cz+1]){
					for(int i=0;i<6;i++){
						chungusSetB(clay,bx+7,by+i+1,bz+i+6,0);
						chungusSetB(clay,bx+8,by+i+1,bz+i+6,0);
						chungusSetB(clay,bx+7,by+i+2,bz+i+6,0);
						chungusSetB(clay,bx+8,by+i+2,bz+i+6,0);
						chungusSetB(clay,bx+7,by+i+3,bz+i+6,0);
						chungusSetB(clay,bx+8,by+i+3,bz+i+6,0);
						chungusSetB(clay,bx+7,by+i+4,bz+i+6,0);
						chungusSetB(clay,bx+8,by+i+4,bz+i+6,0);
					}
				}
			}
		}
	}

}

void worldgenGenerate(worldgen *wgen){
	//static unsigned int averageTicks=0;
	static unsigned int averageRuns=0;

	uint8_t b;
	int oldSeed = getRNGSeed();
	//unsigned int startTicks = getTicks();
	unsigned int seed = (optionWorldSeed | (optionWorldSeed << 16));
	seed ^= (wgen->gx&0xFF00) | (wgen->gx >> 8) | ((wgen->gx&0xFF00)<<8) | ((wgen->gx&0xFF00)<<16);
	seed ^= (wgen->gy&0xFF00) | (wgen->gy >> 8) | ((wgen->gy&0xFF00)<<8) | ((wgen->gy&0xFF00)<<16);
	seed ^= (wgen->gz&0xFF00) | (wgen->gz >> 8) | ((wgen->gz&0xFF00)<<8) | ((wgen->gz&0xFF00)<<16);

	seedRNG(seed);
	if(wgen->geoworld > 188){
		wgen->geoIslands = true;
	}else{
		wgen->geoIslands = false;
	}
	switch(wgen->layer){
		default:
			b=1;
			for(int x=0;x<CHUNK_SIZE;x+=2){
				for(int y=0;y<CHUNK_SIZE;y+=2){
					for(int z=0;z<CHUNK_SIZE;z+=2){
						chungusFill(wgen->clay,x*CHUNK_SIZE,y*CHUNK_SIZE,z*CHUNK_SIZE,b);
						if(++b > 12){b=1;}
					}
				}
			}
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
