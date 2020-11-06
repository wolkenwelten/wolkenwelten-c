#include "island.h"

#include "geoworld.h"
#include "../game/animal.h"
#include "../voxel/chunk.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/world/vegetation.h"

#include <stdlib.h>
#include <string.h>

void worldgenRock(worldgen *wgen,int x,int y,int z,int w,int h,int d){
	int iterations = 12;
	chungus *clay = wgen->clay;

	if((w < 8) || (h < 8) || (d < 8)){
		chungusBoxF(clay,x-w,y-h,z-d,w*2,h*2,d*2,3);
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

typedef struct {
	int  bigTreeChance;
	int  treeChance;
	int  deadTreeChance;

	int  shrubChance;
	int  stoneChance;
	int  dirtChance;

	int  monolithChance;

	int  animalChance;

	int  hematiteChance;
	int  coalChance;
	int  dirtVeinChance;
	int  crystalChance;

	int  lastBlock;
	int  airBlocks;
	int  leafBlocks;

	int  treeType;
	bool hasSpecial;
} wgChances;

static void worldgenCalcChances(const worldgen *wgen, wgChances *w){
	memset(w,0,sizeof(wgChances));
	w->treeType = rngValM(2);

	w->hematiteChance = 512;
	w->coalChance     = 512;
	w->dirtVeinChance = 256;

	if(wgen->layer > 16){
		w->crystalChance  = 1024;
		w->hematiteChance =  256;
		w->coalChance     =  256;
		w->dirtVeinChance =  192;
	}

	switch(wgen->vegetationChance){
		case 7:
			w->bigTreeChance  =   148;
			w->treeChance     =    28;
			w->shrubChance    =    20;
			w->animalChance   =   384;
			break;
		case 6:
			w->bigTreeChance  =   192;
			w->treeChance     =    32;
			w->shrubChance    =    24;
			w->animalChance   =   512;
			break;
		default:
		case 5:
			w->bigTreeChance  =   256;
			w->treeChance     =    48;
			w->shrubChance    =    48;
			w->animalChance   =   768;
			break;
		case 4:
			w->bigTreeChance  =   512;
			w->treeChance     =    96;
			w->shrubChance    =    64;
			w->animalChance   =  1024;
			break;
		case 3:
			w->treeChance     =   768;
			w->shrubChance    =    96;
			w->dirtChance     =    32;
			w->stoneChance    =   256;
			w->animalChance   =  2048;
			break;
		case 2:
			w->treeChance     =  1024;
			w->shrubChance    =    64;
			w->dirtChance     =    12;
			w->stoneChance    =   128;
			w->animalChance   =  4096;
			break;
		case 1:
			w->shrubChance    =   256;
			w->dirtChance     =     4;
			w->stoneChance    =    64;
			w->treeChance     =  4096;
			w->deadTreeChance =  4096;
			w->animalChance   =  8192;
			break;
		case 0:
			w->shrubChance    =  1024;
			w->dirtChance     =     2;
			w->stoneChance    =    32;
			w->monolithChance = 32000;
			w->treeChance     =  8192*2;
			w->deadTreeChance =  8192;
			w->animalChance   =  8192*2;
			break;
	}
}

static inline bool worldgenRDMonolith(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(w->hasSpecial || (!w->monolithChance) || rngValM(w->monolithChance)){return false;}
	worldgenMonolith(wgen,cx,cy,cz);
	w->lastBlock  = I_Obsidian;
	w->airBlocks  = 0;
	w->hasSpecial = true;
	return true;
}

static inline bool worldgenRDBigTree(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->bigTreeChance || (w->airBlocks <= 32) || (rngValM(w->bigTreeChance))){return false;}
	if(w->treeType==0){
		vegBigSpruce(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
	}else if(w->treeType == 1){
		vegBigOak(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
	}else{
		vegBigDeadTree(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
	}
	w->lastBlock = I_Oak;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDTree(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->treeChance || (w->airBlocks <= 16) || rngValM(w->treeChance)){return false;}
	if(w->treeType==0){
		vegSpruce(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
	}else if(w->treeType == 1){
		vegOak(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
	}
	w->lastBlock = I_Oak;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDAnimal(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->animalChance || (w->airBlocks <= 4) || rngValM(w->animalChance)){return false;}
	const vec pos = vecNew(wgen->gx+cx,wgen->gy+cy+2.f,wgen->gz+cz);
	animalNew(pos,1,0);
	animalNew(pos,1,1);
	return true;
}

static inline bool worldgenRDDeadTree(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->deadTreeChance || (w->airBlocks <= 16) || rngValM(w->deadTreeChance)){return false;}
	vegDeadTree(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
	w->lastBlock = I_Oak;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDShrub(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(w->airBlocks <= 8){return false;}
	if(w->shrubChance && (rngValM(w->shrubChance)==12)){
		vegShrub(wgen->gx+cx,wgen->gy+cy,wgen->gz+cz);
		w->lastBlock = I_Dirt;
		w->airBlocks = 0;
		return true;
	}
	if((w->dirtChance) && (rngValM(w->dirtChance)==1)){
		w->lastBlock = I_Dirt;
		w->airBlocks = 0;
		return true;
	}
	if(w->stoneChance && (rngValM(w->stoneChance)==1)){
		chungusSetB(wgen->clay,cx,cy  ,cz,I_Stone);
		chungusSetB(wgen->clay,cx,cy+1,cz,I_Stone);
		return true;
	}
	return false;
}

static inline bool worldgenRDHematite(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->hematiteChance || (w->airBlocks > 0) || rngValM(w->hematiteChance)){return false;}
	const int cw = rngValMM(1,4);
	const int ch = rngValMM(1,4);
	const int cd = rngValMM(1,4);
	chungusBoxF(wgen->clay,cx-cw,cy,cz-cd,cw,ch,cd,I_Hematite_Ore);
	w->lastBlock = I_Hematite_Ore;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDCoal(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->coalChance || (w->airBlocks > 0) || rngValM(w->coalChance)){return false;}
	const int cw = rngValMM(2,5);
	const int ch = rngValMM(2,5);
	const int cd = rngValMM(2,5);
	chungusBoxF(wgen->clay,cx-cw,cy,cz-cd,cw,ch,cd,I_Coal);
	w->lastBlock = I_Coal;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDDirt(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->dirtVeinChance || (w->airBlocks > 0) || rngValM(w->dirtVeinChance)){return false;}
	const int cw = rngValMM(2,6);
	const int ch = rngValMM(2,6);
	const int cd = rngValMM(2,6);
	chungusBoxF(wgen->clay,cx-cw,cy,cz-cd,cw,ch,cd,I_Dirt);
	w->lastBlock = I_Dirt;
	w->airBlocks = 0;
	return true;
}

static void worldgenRDFirstPass(worldgen *wgen, wgChances *w){
	chungus *clay = wgen->clay;
	for(int cz=wgen->minZ;cz<=wgen->maxZ;cz++){
		int z = cz&0xF;
		for(int cx=wgen->minX;cx<=wgen->maxX;cx++){
			int x = cx&0xF;
			w->airBlocks = 8;
			w->lastBlock = 0;
			chunk *chnk = NULL;
			for(int cy=wgen->maxY;cy>=wgen->minY;cy--){
				if((chnk == NULL) || ((cy&0xF)==0xF)){
					chnk = clay->chunks[cx>>4][cy>>4][cz>>4];
				}
				if(chnk == NULL){
					w->lastBlock = 0;
					w->airBlocks += cy - (cy&(~0xF));
					cy = cy & (~0xF);
					continue;
				}
				u8 b = chnk->data[x][cy&0xF][z];
				switch(b){
					default:
						w->airBlocks = 0;
					break;

					case I_Oak_Leaf:
					case I_Spruce_Leaf:
					case I_Sakura_Leaf:
					case 0:
						w->airBlocks++;
						break;
					case I_Stone:
						w->airBlocks = 0;
						if(worldgenRDHematite(wgen,w,cx,cy,cz)){continue;}
						if(worldgenRDCoal    (wgen,w,cx,cy,cz)){continue;}
						if(worldgenRDDirt    (wgen,w,cx,cy,cz)){continue;}
						break;
					case I_Dirt:
						if(worldgenRDMonolith(wgen,w,cx,cy,cz)){continue;}
						if(worldgenRDBigTree (wgen,w,cx,cy,cz)){continue;}
						if(worldgenRDTree    (wgen,w,cx,cy,cz)){continue;}
						if(worldgenRDAnimal  (wgen,w,cx,cy,cz)){continue;}
						if(worldgenRDDeadTree(wgen,w,cx,cy,cz)){continue;}
					break;
				}
				w->lastBlock = b;
			}
		}
	}
}

void worldgenRemoveDirt(worldgen *wgen){
	chungus *clay = wgen->clay;
	wgChances w;
	worldgenCalcChances(wgen,&w);

	if(wgen->minX < 0){wgen->minX = 0;}
	if(wgen->minY < 0){wgen->minY = 0;}
	if(wgen->minZ < 0){wgen->minZ = 0;}
	if(wgen->maxX >= CHUNGUS_SIZE){wgen->maxX = CHUNGUS_SIZE-1;}
	if(wgen->maxY >= CHUNGUS_SIZE){wgen->maxY = CHUNGUS_SIZE-1;}
	if(wgen->maxZ >= CHUNGUS_SIZE){wgen->maxZ = CHUNGUS_SIZE-1;}

	worldgenRDFirstPass(wgen,&w);

	for(int cz=wgen->minZ;cz<=wgen->maxZ;cz++){
		int z = cz&0xF;
		for(int cx=wgen->minX;cx<=wgen->maxX;cx++){
			int x = cx&0xF;
			w.airBlocks  = 8;
			w.lastBlock  = 0;
			w.leafBlocks = 0;
			chunk *chnk  = NULL;
			for(int cy=wgen->maxY;cy>=wgen->minY;cy--){
				if((chnk == NULL) || ((cy&0xF)==0xF)){
					chnk = clay->chunks[cx>>4][cy>>4][cz>>4];
				}
				if(chnk == NULL){
					w.lastBlock = 0;
					w.airBlocks += cy - (cy&(~0xF));
					cy = cy & (~0xF);
					continue;
				}
				u8 b = chnk->data[x][cy&0xF][z];
				switch(b){
					default:
						w.leafBlocks = w.airBlocks = 0;
						break;

					case I_Oak_Leaf:
					case I_Spruce_Leaf:
					case I_Sakura_Leaf:
						w.leafBlocks++; /* Fall through */
					case 0:
						w.airBlocks++;
						break;
					case I_Dirt:
					case I_Stone:
					case I_Hematite_Ore:
					case I_Coal:
					case I_Crystal:
						if(!w.airBlocks)                      {continue;}
						if(worldgenRDShrub(wgen,&w,cx,cy,cz)) {continue;}
						if((w.airBlocks > 8) && (w.leafBlocks < 3)){
							chnk->data[x][cy&0xF][z] = I_Grass;
						}else if(w.airBlocks > 3){
							chnk->data[x][cy&0xF][z] = I_Dirt;
						}
						w.airBlocks = 0;
					break;
				}
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
