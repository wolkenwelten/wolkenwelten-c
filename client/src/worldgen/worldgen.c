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
#include "worldgen.h"

#include "../../../common/src/game/blockType.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

#include <stdlib.h>
#include <math.h>

static void wgShrub(chungus *c, int x,int y,int z){
	const int leafBlock      = I_Oak_Leaf;
	const int otherLeafBlock = I_Flower;
	const int woodBlock      = I_Oak;
	chungusSetB(c,x,y-1,z,I_Roots);
	chungusSetB(c,x,y  ,z,I_Roots);
	chungusSetB(c,x,y+1,z,woodBlock);

	for(int ox = -1;ox<2;ox++){
	for(int oy =  2;oy<4;oy++){
	for(int oz = -1;oz<2;oz++){
		if(rngValA(15) == 0){continue;}
		const blockId b = rngValA(7) != 0 ? leafBlock : otherLeafBlock;
		chungusSetB(c,x+ox,y+oy,z+oz,b);
	}
	}
	}
	chungusSetB(c,x,y+2,z,woodBlock);
	chungusSetB(c,x,y+3,z,leafBlock);
	chungusSetB(c,x,y+4,z,leafBlock);
}

static void wgBush(chungus *c, int x,int y,int z){
	chungusSetB(c,x,y  ,z,I_Roots);
	chungusSetB(c,x,y+1,z,I_Flower);
	if(rngValA(7) == 1){
		chungusSetB(c,x,y+2,z,I_Flower);
	}
}

static void wgRoots(chungus *c, int x,int y,int z){
	const int size = rngValMM(4,12);
	for(int cy = 0;cy > -size;--cy){
		blockId b = chungusGetB(c,x,y+cy,z);
		switch(b){
			case 0:
			case I_Roots:
			case I_Dirt:
			case I_Grass:
				chungusSetB(c,x,y+cy,z,I_Roots);
			break;

			default:
				return;
		}
		b = chungusGetB(c,x-1,y+cy,z);
		if(((b == I_Dirt) || (b == I_Grass)) && (rngValM(2)==0)){
			chungusSetB(c,x-1,y+cy,z,I_Roots);
		}
		b = chungusGetB(c,x+1,y+cy,z);
		if(((b == I_Dirt) || (b == I_Grass)) && (rngValM(2)==0)){
			chungusSetB(c,x+1,y+cy,z,I_Roots);
		}
		b = chungusGetB(c,x,y+cy,z-1);
		if(((b == I_Dirt) || (b == I_Grass)) && (rngValM(2)==0)){
			chungusSetB(c,x,y+cy,z-1,I_Roots);
		}
		b = chungusGetB(c,x,y+cy,z+1);
		if(((b == I_Dirt) || (b == I_Grass)) && (rngValM(2)==0)){
			chungusSetB(c,x,y+cy,z+1,I_Roots);
		}
	}
}

static void wgSurroundWithLeafes(chungus *c, int x, int y, int z, blockId leafB){
	for(int cx=-1;cx<=1;cx++){
	for(int cy= 0;cy<=1;cy++){
	for(int cz=-1;cz<=1;cz++){
		if(chungusGetB(c,x+cx,y+cy,z+cz) != 0){continue;}
		chungusSetB(c,x+cx,y+cy,z+cz,leafB);
	}
	}
	}
}

static void wgTree(chungus *c, int x, int y, int z, int logblock, int leafes){
	const int size       = rngValA(7)+12;
	const int sparseness = rngValA(3)+3;
	int lsize;

	for(int cy = 0;cy < size;cy++){
		if(cy >= size-2){lsize=2;}
		else if(cy < 10){lsize=2;}
		else {lsize = 3;}
		if(cy >= 8){
			for(int cz = -lsize;cz<=lsize;cz++){
			for(int cx = -lsize;cx<=lsize;cx++){
				if((cx == 0) && (cz == 0)){
					chungusSetB(c,cx+x,cy+y,cz+z,leafes);
					continue;
				}
				if((cx == -lsize) && (cz == -lsize  )){continue;}
				if((cx == -lsize) && (cz ==  lsize  )){continue;}
				if((cx ==  lsize) && (cz == -lsize  )){continue;}
				if((cx ==  lsize) && (cz ==  lsize  )){continue;}
				if((rngValM(sparseness)) == 0)        {continue;}
				chungusSetB(c,cx+x,cy+y,cz+z,leafes);
			}
			}
		}
		if(cy < size-2){
			chungusSetB(c,x,cy+y,z,logblock);
			if(cy > 3){
				switch(rngValM(8)){
				case 1:
					chungusSetB(c,x+1,cy+y,z,logblock);
					wgSurroundWithLeafes(c,x+1,cy+y,z,leafes);
					if(rngValM(4) == 0){
						chungusSetB(c,x+2,cy+y,z,logblock);
						wgSurroundWithLeafes(c,x+2,cy+y,z,leafes);
					}
					break;
				case 2:
					chungusSetB(c,x-1,cy+y,z,logblock);
					wgSurroundWithLeafes(c,x-1,cy+y,z,leafes);
					if(rngValM(4) == 0){
						chungusSetB(c,x-2,cy+y,z,logblock);
						wgSurroundWithLeafes(c,x-2,cy+y,z,leafes);
					}
					break;
				case 3:
					chungusSetB(c,x,cy+y,z+1,logblock);
					wgSurroundWithLeafes(c,x,cy+y,z+1,leafes);
					if(rngValM(4) == 0){
						chungusSetB(c,x,cy+y,z+2,logblock);
						wgSurroundWithLeafes(c,x,cy+y,z+2,leafes);
					}
					break;
				case 4:
					chungusSetB(c,x,cy+y,z-1,logblock);
					wgSurroundWithLeafes(c,x,cy+y,z-1,leafes);
					if(rngValM(4) == 0){
						chungusSetB(c,x,cy+y,z-2,logblock);
						wgSurroundWithLeafes(c,x,cy+y,z-2,leafes);
					}
					break;
				}
			}
		}
	}
	wgRoots(c,x,y-1,z);
}

void wgOak(chungus *c, int x,int y,int z){
	wgTree(c,x,y,z,I_Oak,I_Oak_Leaf);
}
void wgBirch(chungus *c, int x,int y,int z){
	wgTree(c,x,y,z,I_Birch,I_Oak_Leaf);
}
void wgSakura(chungus *c, int x,int y,int z){
	wgTree(c,x,y,z,I_Oak,I_Sakura_Leaf);
}

chungus *worldGenChungus(chungus *chng){
	const int cy = chng->y<<8;
	const int cx = (chng->x<<8);
	const int cz = (chng->z<<8);
	const int cxd = (1<<15) - cx;
	const int czd = (1<<15) - cz;
	const float cd = sqrtf(cxd*cxd + czd*czd);
	if(cy != 256){return chng;}
	if(cd > 600){return chng;}

	for(int x = 0;x<256;x++){
	for(int z = 0;z<256;z++){
		const int xd = (1<<15) - (cx+x);
		const int zd = (1<<15) - (cz+z);
		const float d = xd*xd + zd*zd;
		if(d > (128*128)){continue;}
		const int sy = 128;
		int h = 130-sqrtf(d);
		if(h < 1){h = 1;}
		if(h > 24){h = 24;}

		int th = (130-sqrtf(d))/15;

		chungusBox(chng, x, th+sy-h, z, 1, h, 1, 1);
		chungusBox(chng, x, th+sy, z, 1, 1, 1, 2);

		if(d < (96*96)){
			if(rngValA(0xFF) == 0){
				chungusBox(chng, x, th+sy, z, rngValMM(1, 3), rngValMM(1, 3), rngValMM(1, 3), 3);
			}
			else if(rngValA(0x1FF) == 0){
				wgOak(chng, x, th+sy, z);
			}
			else if(rngValA(0x1FF) == 0){
				wgBirch(chng, x, th+sy, z);
			}
			else if(rngValA(0x1FF) == 0){
				wgSakura(chng, x, th+sy, z);
			}
			else if(rngValA(0x7F) == 0){
				wgBush(chng, x, th+sy, z);
			}
			else if(rngValA(0x1FF) == 0){
				wgShrub(chng, x, th+sy, z);
			}
		}
	}
	}

	return chng;
}
