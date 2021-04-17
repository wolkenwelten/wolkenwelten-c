/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "island.h"

#include "geoworld.h"
#include "vegetation.h"
#include "../game/animal.h"
#include "../voxel/chunk.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <stdlib.h>
#include <string.h>

void worldgenRock(worldgen *wgen,int x,int y,int z,int w,int h,int d){
	int iterations = 6;
	chungus *clay = wgen->clay;

	if((w < 2) || (h < 2) || (d < 2)){return;}
	if((w < 8) || (h < 8) || (d < 8)){
		chungusBoxFWG(clay,x-w,y-h,z-d,w*2,h*2,d*2);
		wgen->minX = MIN(wgen->minX,x-w);
		wgen->minY = MIN(wgen->minY,y-h);
		wgen->minZ = MIN(wgen->minZ,z-d);
		wgen->maxX = MAX(wgen->maxX,x+w);
		wgen->maxY = MAX(wgen->maxY,y+h);
		wgen->maxZ = MAX(wgen->maxZ,z+d);
	}

	for(int i=0;i<iterations;i++){
		int nx,ny,nz;
		int nw = w/2;
		int nh = h/2;
		int nd = d/2;
		if(wgen->iterChance > 0){
			switch(rngValM(wgen->iterChance * 8)){
			case 0:
				nw = nw << 1;
				break;
			case 1:
				nh = nh >> 1;
				break;
			case 2:
				nd = nd << 1;
				break;
			case 3:
				nw = nw >> 1;
				break;
			case 4:
				nh = nh >> 1;
				break;
			case 5:
				nd = nd >> 1;
				break;
			}
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
			ny = y - h;
			nz = rngValMM(z-d,z+d);
		break;
		case 3:
			nx = rngValMM(x-w,x+w);
			ny = y + h;
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
			nz = z + d + (rngValM(d)-d/2);
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
	int  bushChance;
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
	u8   grassBlock;
	bool hasSpecial;
} wgChances;

static void worldgenCalcChances(const worldgen *wgen, wgChances *w){
	memset(w,0,sizeof(wgChances));
	w->grassBlock = I_Grass;

	w->hematiteChance = 2047;
	w->coalChance     = 2047;
	w->dirtVeinChance = 1023;

	if(wgen->layer > 16){
		w->crystalChance  = 1023;
		w->hematiteChance =  255;
		w->coalChance     =  255;
		w->dirtVeinChance =  255;
	}

	switch(wgen->vegetationConcentration){
		case 7:
			w->treeType       = 0;
			w->bigTreeChance  = (1<< 6)-1;
			w->treeChance     = (1<< 5)-1;
			w->shrubChance    = (1<< 4)-1;
			w->animalChance   = (1<< 9)-1;
			break;
		case 6:
			w->treeType       = 0;
			w->bigTreeChance  = (1<< 6)-1;
			w->treeChance     = (1<< 5)-1;
			w->shrubChance    = (1<< 4)-1;
			w->animalChance   = (1<< 9)-1;
			break;
		default:
		case 5:
			w->treeType       = 1;
			w->bigTreeChance  = (1<< 7)-1;
			w->treeChance     = (1<< 6)-1;
			w->shrubChance    = (1<< 5)-1;
			w->animalChance   = (1<<10)-1;
			break;
		case 4:
			w->treeType       = 1;
			w->bigTreeChance  = (1<< 9)-1;
			w->treeChance     = (1<< 9)-1;
			w->shrubChance    = (1<< 6)-1;
			w->animalChance   = (1<<11)-1;
			break;
		case 3:
			w->treeType       = 2;
			w->bigTreeChance  = (1<<11)-1;
			w->treeChance     = (1<<10)-1;
			w->shrubChance    = (1<< 5)-1;
			w->dirtChance     = (1<< 5)-1;
			w->stoneChance    = (1<< 8)-1;
			w->animalChance   = (1<<12)-1;
			w->grassBlock     = I_Dry_Grass;
			break;
		case 2:
			w->treeType       = 2;
			w->bigTreeChance  = (1<<13)-1;
			w->treeChance     = (1<<12)-1;
			w->shrubChance    = (1<< 6)-1;
			w->dirtChance     = (1<< 4)-1;
			w->stoneChance    = (1<< 7)-1;
			w->animalChance   = (1<<13)-1;
			w->grassBlock     = I_Dry_Grass;
			break;
		case 1:
			w->treeType       = 5;
			w->shrubChance    = (1<< 7)-1;
			w->dirtChance     = (1<< 2)-1;
			w->stoneChance    = (1<< 5)-1;
			w->bigTreeChance  = (1<<14)-1;
			w->treeChance     = (1<<12)-1;
			w->animalChance   = (1<<14)-1;
			w->grassBlock     = I_Dry_Grass;
			break;
		case 0:
			w->treeType       = 5;
			w->shrubChance    = (1<< 9)-1;
			w->dirtChance     = (1<< 1)-1;
			w->stoneChance    = (1<< 5)-1;
			w->monolithChance = (1<<18)-1;
			w->bigTreeChance  = (1<<15)-1;
			w->treeChance     = (1<<14)-1;
			w->animalChance   = (1<<15)-1;
			w->grassBlock     = I_Dry_Grass;
			break;
	}
	w->bushChance = w->shrubChance >> 1;

	if(w->treeType == 1){
		switch(rngValA(31)){
		case 0:
			w->treeType = 3;
			w->treeChance = w->treeChance >> 2;
			break;
		case 1:
			w->treeType = 4;
			w->treeChance = w->treeChance >> 2;
			break;
		default:
			break;
		}
	}
}

static inline bool worldgenRDMonolith(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(w->hasSpecial || (!w->monolithChance) || rngValA(w->monolithChance)){return false;}
	worldgenMonolith(wgen,cx,cy,cz);
	w->lastBlock  = I_Obsidian;
	w->airBlocks  = 0;
	w->hasSpecial = true;
	return true;
}

static inline bool worldgenRDBigTree(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->bigTreeChance || (w->airBlocks <= 32) || rngValA(w->bigTreeChance)){return false;}
	switch(w->treeType){
	default:
	case 0:
		wgBigSpruce(wgen->clay,cx,cy,cz);
		break;
	case 1:
		wgBigOak(wgen->clay,cx,cy,cz);
		break;
	case 2:
		wgBigAcacia(wgen->clay,cx,cy,cz);
		break;
	case 3:
		wgBigBirch(wgen->clay,cx,cy,cz);
		break;
	case 4:
		wgBigSakura(wgen->clay,cx,cy,cz);
		break;
	case 5:
		wgBigDeadTree(wgen->clay,cx,cy,cz);
		break;
	}
	w->lastBlock = I_Oak;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDTree(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->treeChance || (w->airBlocks <= 16) || rngValA(w->treeChance)){return false;}
	switch(w->treeType){
	default:
	case 0:
		wgSpruce(wgen->clay,cx,cy,cz);
		break;
	case 1:
		wgOak(wgen->clay,cx,cy,cz);
		break;
	case 2:
		wgAcacia(wgen->clay,cx,cy,cz);
		break;
	case 3:
		wgBirch(wgen->clay,cx,cy,cz);
		break;
	case 4:
		wgSakura(wgen->clay,cx,cy,cz);
		break;
	case 5:
		wgDeadTree(wgen->clay,cx,cy,cz);
		break;
	}
	w->lastBlock = I_Oak;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDAnimal(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->animalChance || (w->airBlocks <= 4) || rngValA(w->animalChance)){return false;}
	const vec pos = vecNew(wgen->gx+cx,wgen->gy+cy+2.f,wgen->gz+cz);
	if(rngValA(15) == 0){
		animalNew(pos,3,rngValA(1));
	}else{
		animalNew(pos,1,0);
		animalNew(pos,1,1);
	}
	return true;
}

static inline bool worldgenRDDeadTree(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->deadTreeChance || (w->airBlocks <= 16) || rngValA(w->deadTreeChance)){return false;}
	wgDeadTree(wgen->clay,cx,cy,cz);
	w->lastBlock = I_Oak;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDShrub(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(w->airBlocks <= 8){return false;}
	if(rngValA(w->bushChance)==1){
		wgBush(wgen->clay,cx,cy,cz);
		w->lastBlock = I_Dirt;
		w->airBlocks = 0;
		return true;
	}
	if(rngValA(w->shrubChance)==1){
		if(w->treeType == 2){
			wgDate(wgen->clay,cx,cy,cz);
		}else{
			wgShrub(wgen->clay,cx,cy,cz);
		}
		w->lastBlock = I_Dirt;
		w->airBlocks = 0;
		return true;
	}
	if((rngValA(w->dirtChance)==1)){
		w->lastBlock = I_Dirt;
		w->airBlocks = 0;
		return true;
	}
	if(rngValA(w->stoneChance)==1){
		chungusSetB(wgen->clay,cx,cy  ,cz,I_Stone);
		chungusSetB(wgen->clay,cx,cy+1,cz,I_Stone);
		return true;
	}
	return false;
}

static inline bool worldgenRDHematite(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->hematiteChance || (w->airBlocks > 0) || rngValA(w->hematiteChance)){return false;}
	const int cw = (rngValR()&3)+1;
	const int ch = (rngValR()&3)+1;
	const int cd = (rngValR()&3)+1;
	if((cx-cw) < wgen->minX){return false;}
	if((cy-ch) < wgen->minY){return false;}
	if((cz-cd) < wgen->minZ){return false;}
	chungusBoxF(wgen->clay,cx-cw,cy-ch,cz-cd,cw,ch,cd,I_Hematite_Ore);
	w->lastBlock = I_Hematite_Ore;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDCoal(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->coalChance || (w->airBlocks > 0) || rngValA(w->coalChance)){return false;}
	const int cw = (rngValR()&3)+2;
	const int ch = (rngValR()&3)+2;
	const int cd = (rngValR()&3)+2;
	if((cx-cw) < wgen->minX){return false;}
	if((cy-ch) < wgen->minY){return false;}
	if((cz-cd) < wgen->minZ){return false;}
	chungusBoxF(wgen->clay,cx-cw,cy-ch,cz-cd,cw,ch,cd,I_Coal);
	w->lastBlock = I_Coal;
	w->airBlocks = 0;
	return true;
}

static inline bool worldgenRDDirt(worldgen *wgen, wgChances *w, int cx, int cy, int cz){
	if(!w->dirtVeinChance || (w->airBlocks > 0) || rngValA(w->dirtVeinChance)){return false;}
	const int cw = (rngValR()&3)+3;
	const int ch = (rngValR()&3)+3;
	const int cd = (rngValR()&3)+3;
	if((cx-cw) < wgen->minX){return false;}
	if((cy-ch) < wgen->minY){return false;}
	if((cz-cd) < wgen->minZ){return false;}
	chungusBoxF(wgen->clay,cx-cw,cy-ch,cz-cd,cw,ch,cd,I_Dirt);
	w->lastBlock = I_Dirt;
	w->airBlocks = 0;
	return true;
}

static void worldgenRDFirstPass(worldgen *wgen, wgChances *w){
	chungus *clay = wgen->clay;
	for(int cx=wgen->minX;cx<=wgen->maxX;cx++){
		int x = cx&0xF;
		for(int cz=wgen->minZ;cz<=wgen->maxZ;cz++){
			int z = cz&0xF;
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

	for(int cx=wgen->minX;cx<=wgen->maxX;cx++){
		int x = cx&0xF;
		for(int cz=wgen->minZ;cz<=wgen->maxZ;cz++){
			int z = cz&0xF;
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
						chnk->data[x][cy&0xF][z] = w.grassBlock;
					}else if(w.airBlocks > 3){
						chnk->data[x][cy&0xF][z] = I_Dirt;
					}
					if(worldgenRDAnimal  (wgen,&w,cx,cy,cz)){continue;}
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

void worldgenSpawnIsland(worldgen *wgen, int x,int y,int z,int size){
	worldgenIsland(wgen,x,y,z,size);
	worldgenFindSpawn(wgen,x,z,0);
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

	int xoff = rngValM(MAX(CHUNGUS_SIZE/4,CHUNGUS_SIZE-(size*4)))-(CHUNGUS_SIZE-(size*4))/2;
	int yoff = rngValM(MAX(CHUNGUS_SIZE/4,CHUNGUS_SIZE-(size*4)))-(CHUNGUS_SIZE-(size*4))/2;
	int zoff = rngValM(MAX(CHUNGUS_SIZE/4,CHUNGUS_SIZE-(size*4)))-(CHUNGUS_SIZE-(size*4))/2;
	const uint roll = rngValA(3);
	if(roll == 0){
		worldgenSpawnIsland(wgen,CHUNGUS_SIZE/2+xoff,CHUNGUS_SIZE/2+yoff,CHUNGUS_SIZE/2+zoff,size);
	}else{
		iMin*=2;
	}
	if(wgen->geoIslands){
		iMin *= 12;
		iMax *= 12;
		iSize /= 2;
	}
	iMax = MAX(iMax,iMin+1);
	for(int i=rngValMM(iMin,iMax);i>0;i--){
		int nx = rngValM(CHUNGUS_SIZE-CHUNGUS_SIZE/8)+CHUNGUS_SIZE/16;
		int ny = rngValM(CHUNGUS_SIZE-CHUNGUS_SIZE/8)+CHUNGUS_SIZE/16;
		int nz = rngValM(CHUNGUS_SIZE-CHUNGUS_SIZE/8)+CHUNGUS_SIZE/16;
		worldgenIsland(wgen,nx,ny,nz,iSize);
	}
}
