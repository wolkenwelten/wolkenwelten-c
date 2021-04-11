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

#include "vegetation.h"

#include "../../../common/src/game/item.h"
#include "../voxel/chungus.h"
#include "../../../common/src/misc/misc.h"

void wgShrub(chungus *c, int x,int y,int z){
	int leafBlock = I_Oak_Leaf;
	int otherLeafBlock = I_Flower;
	int woodBlock = I_Oak;
	chungusSetB(c,x,y-1,z,I_Roots);
	chungusSetB(c,x,y  ,z,I_Roots);
	chungusSetB(c,x,y+1,z,woodBlock);

	for(int ox = -1;ox<2;ox++){
	for(int oy =  2;oy<4;oy++){
	for(int oz = -1;oz<2;oz++){
		if(rngValA(15) == 0){continue;}
		chungusSetB(c,x+ox,y+oy,z+oz,rngValA(7) != 0 ? leafBlock : otherLeafBlock);
	}
	}
	}
	chungusSetB(c,x,y+2,z,woodBlock);
	chungusSetB(c,x,y+3,z,leafBlock);
	chungusSetB(c,x,y+4,z,leafBlock);
}

void wgBush(chungus *c, int x,int y,int z){
	chungusSetB(c,x,y  ,z,I_Roots);
	chungusSetB(c,x,y+1,z,I_Flower);
	if(rngValA(7) == 1){
		chungusSetB(c,x,y+2,z,I_Flower);
	}
}

void wgDate(chungus *c, int x,int y,int z){
	int leaveBlock = I_Date;
	chungusSetB(c,x,y,z,I_Roots);
	chungusSetB(c,x,y+1,z,leaveBlock);
}

void wgRoots(chungus *c, int x,int y,int z){
	int size = rngValMM(4,12);
	for(int cy = 0;cy > -size;--cy){
		u8 b = chungusGetB(c,x,y+cy,z);
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

void wgBigRoots(chungus *c, int x,int y,int z){
	for(int cx = 0;cx < 2;cx++){
		for(int cz = 0;cz < 2;cz++){
			wgRoots(c,cx+x,y,cz+z);
		}
	}
}

void wgDeadTree(chungus *c, int x,int y,int z){
	int size       = rngValMM(12,16);
	for(int cy = 0;cy < size;cy++){
		chungusSetB(c,x,cy+y,z,5);
	}
	wgRoots(c,x,y-1,z);
}

void wgBigDeadTree(chungus *c, int x,int y,int z){
	int size       = rngValMM(20,34);
	for(int cy = -5;cy < size;cy++){
		if(cy < -2){
			chungusSetB(c,x  ,cy+y,z  ,8);
			chungusSetB(c,x+1,cy+y,z  ,8);
			chungusSetB(c,x  ,cy+y,z+1,8);
			chungusSetB(c,x+1,cy+y,z+1,8);
		}else{
			chungusSetB(c,x  ,cy+y,z  ,5);
			chungusSetB(c,x+1,cy+y,z  ,5);
			chungusSetB(c,x  ,cy+y,z+1,5);
			chungusSetB(c,x+1,cy+y,z+1,5);
		}
	}
	wgBigRoots(c,x,y-5,z);
}

void wgSpruce(chungus *c, int x,int y,int z){
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
					chungusSetB(c,cx+x,cy+y,cz+z,6);
				}
			}
		}
		chungusSetB(c,x,cy+y,z,5);
	}
	chungusSetB(c,x,size+y  ,z,6);
	chungusSetB(c,x,size+y+1,z,6);
	wgRoots(c,x,y-1,z);
}

void wgBigSpruce(chungus *c, int x,int y,int z){
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
					chungusSetB(c,cx+x,cy+y,cz+z,6);
				}
			}
		}
		if(cy < -2){
			chungusSetB(c,x  ,cy+y,z  ,8);
			chungusSetB(c,x+1,cy+y,z  ,8);
			chungusSetB(c,x  ,cy+y,z+1,8);
			chungusSetB(c,x+1,cy+y,z+1,8);
		}else{
			chungusSetB(c,x  ,cy+y,z  ,5);
			chungusSetB(c,x+1,cy+y,z  ,5);
			chungusSetB(c,x  ,cy+y,z+1,5);
			chungusSetB(c,x+1,cy+y,z+1,5);
		}
	}
	for(int cx=0;cx<2;cx++){
		for(int cy=0;cy<2;cy++){
			for(int cz=0;cz<2;cz++){
				chungusSetB(c,x+cx,size+y+cy,z+cz,6);
			}
		}
	}
	wgBigRoots(c,x,y-5,z);
}

void wgSurroundWithLeafes(chungus *c, int x, int y, int z, u8 leafB){
	for(int cx=-1;cx<=1;cx++){
		for(int cy=0;cy<=1;cy++){
			for(int cz=-1;cz<=1;cz++){
				if(chungusGetB(c,x+cx,y+cy,z+cz) != 0){continue;}
				chungusSetB(c,x+cx,y+cy,z+cz,leafB);
			}
		}
	}
}

void wgAcacia(chungus *c, int x, int y, int z){
	int size       = rngValA(7)+10;
	int sparseness = rngValA(3)+3;
	int lsize      = 0;
	int logblock   = I_Spruce;
	int leafes     = I_Acacia_Leaf;
	int r;

	for(int cy = 0;cy < size;cy++){
		if(cy == size-1){lsize=2;}
		else if(cy == size-2){lsize=5;}
		else if(cy == size-3){lsize=1;}
		if(cy >= 5){
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
				r = rngValM(8);
				switch(r){
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

static void wgTree(chungus *c, int x, int y, int z, int logblock, int leafes){
	int size       = rngValA(7)+12;
	int sparseness = rngValA(3)+3;
	int lsize      = 4;
	int r;

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
				r = rngValM(8);
				switch(r){
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

void wgBigTree(chungus *c, int x,int y,int z, int logblock, int leafes){
	int size       = rngValA(15)+12;
	int sparseness = rngValA( 3)+4;
	int lsize      = 5;

	for(int cy = -5;cy < size;cy++){
		lsize = (cy-9);
		if((size - cy) < lsize){
			lsize = size-cy;
		}
		if(cy >= 12){
			for(int cz = -lsize;cz<=lsize+1;cz++){
				for(int cx = -lsize;cx<=lsize+1;cx++){
					if((cx == -lsize  ) && (cz == -lsize  )){continue;}
					if((cx == -lsize  ) && (cz ==  lsize+1)){continue;}
					if((cx ==  lsize+1) && (cz == -lsize  )){continue;}
					if((cx ==  lsize+1) && (cz ==  lsize+1)){continue;}
					if(rngValM(sparseness) == 0)            {continue;}
					chungusSetB(c,cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < -2){
			chungusSetB(c,x  ,cy+y,z  ,8);
			chungusSetB(c,x+1,cy+y,z  ,8);
			chungusSetB(c,x  ,cy+y,z+1,8);
			chungusSetB(c,x+1,cy+y,z+1,8);
		}else if(cy < size-2){
			chungusSetB(c,x  ,cy+y,z  ,logblock);
			chungusSetB(c,x+1,cy+y,z  ,logblock);
			chungusSetB(c,x  ,cy+y,z+1,logblock);
			chungusSetB(c,x+1,cy+y,z+1,logblock);
		}
	}
	wgBigRoots(c,x,y-5,z);
}

void wgBigOak(chungus *c, int x,int y,int z){
	wgBigTree(c,x,y,z,I_Oak,I_Oak_Leaf);
}

void wgBigBirch(chungus *c, int x,int y,int z){
	wgBigTree(c,x,y,z,I_Birch,I_Oak_Leaf);
}

void wgBigSakura(chungus *c, int x,int y,int z){
	wgBigTree(c,x,y,z,I_Oak,I_Sakura_Leaf);
}

void wgBigAcacia(chungus *c, int x,int y,int z){
	int size       = rngValA(7) + 18;
	int sparseness = rngValA(3) + 3;
	int lsize      = 8;
	int logblock   = I_Spruce;
	int leafes     = I_Acacia_Leaf;

	for(int cy = -5;cy < size;cy++){
		switch(size - cy){
		case 0:
		default:
			lsize = 0;
			break;
		case 1:
			lsize = 2;
			break;
		case 2:
			lsize = 12;
			break;
		case 3:
			lsize = 8;
			break;
		case 4:
			lsize = 1;
			break;
		}
		if((size - cy) < lsize){
			lsize = size-cy;
		}
		if(cy >= (size - 5)){
			for(int cz = -lsize;cz<=lsize+1;cz++){
				for(int cx = -lsize;cx<=lsize+1;cx++){
					if((cx == -lsize  ) && (cz == -lsize  )){continue;}
					if((cx == -lsize  ) && (cz ==  lsize+1)){continue;}
					if((cx ==  lsize+1) && (cz == -lsize  )){continue;}
					if((cx ==  lsize+1) && (cz ==  lsize+1)){continue;}
					if(rngValM(sparseness) == 0)            {continue;}
					chungusSetB(c,cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < -2){
			chungusSetB(c,x  ,cy+y,z  ,8);
			chungusSetB(c,x+1,cy+y,z  ,8);
			chungusSetB(c,x  ,cy+y,z+1,8);
			chungusSetB(c,x+1,cy+y,z+1,8);
		}else if(cy < size-2){
			chungusSetB(c,x  ,cy+y,z  ,logblock);
			chungusSetB(c,x+1,cy+y,z  ,logblock);
			chungusSetB(c,x  ,cy+y,z+1,logblock);
			chungusSetB(c,x+1,cy+y,z+1,logblock);
		}
	}
	wgBigRoots(c,x,y-5,z);
}
