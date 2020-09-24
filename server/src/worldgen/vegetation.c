#include "vegetation.h"

#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

void worldgenShrub(worldgen *wgen,int x,int y,int z){
	chungus *clay = wgen->clay;
	int leaveBlock = 21;
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
		u8 b = chungusGetB(clay,x,y+cy,z);
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
			chungusSetB(clay,x  ,cy+y,z  ,8);
			chungusSetB(clay,x+1,cy+y,z  ,8);
			chungusSetB(clay,x  ,cy+y,z+1,8);
			chungusSetB(clay,x+1,cy+y,z+1,8);
		}else{
			chungusSetB(clay,x  ,cy+y,z  ,5);
			chungusSetB(clay,x+1,cy+y,z  ,5);
			chungusSetB(clay,x  ,cy+y,z+1,5);
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
	chungusSetB(clay,x,size+y  ,z,6);
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
			chungusSetB(clay,x  ,cy+y,z  ,8);
			chungusSetB(clay,x+1,cy+y,z  ,8);
			chungusSetB(clay,x  ,cy+y,z+1,8);
			chungusSetB(clay,x+1,cy+y,z+1,8);
		}else{
			chungusSetB(clay,x  ,cy+y,z  ,5);
			chungusSetB(clay,x+1,cy+y,z  ,5);
			chungusSetB(clay,x  ,cy+y,z+1,5);
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

void worldgenSurroundWithLeafes(worldgen *wgen, int x, int y, int z, int leafes){
	for(int cx=-1;cx<=1;cx++){
		for(int cy=0;cy<=1;cy++){
			for(int cz=-1;cz<=1;cz++){
				if(chungusGetB(wgen->clay,x+cx,y+cy,z+cz) == 0){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,leafes);
				}
			}
		}
	}
}

void worldgenOak(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(8,12);
	int sparseness = rngValMM(2,3);
	int lsize      = 3;
	int leafes     = 11;
	int logblock   = 10;
	int r;
	if(rngValM(16) == 0){leafes   = 19;} // We Sakura now
	if(rngValM(16) == 0){logblock = 20;} // We Birch now

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
					if((cx == -lsize) && (cz == -lsize  )){continue;}
					if((cx == -lsize) && (cz ==  lsize  )){continue;}
					if((cx ==  lsize) && (cz == -lsize  )){continue;}
					if((cx ==  lsize) && (cz ==  lsize  )){continue;}
					if((rngValM(sparseness)) == 0)        {continue;}
					chungusSetB(clay,cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < size-2){
			chungusSetB(clay,x,cy+y,z,logblock);
			if(cy > 3){
				r = rngValM(8);
				switch(r){
					case 1:
						chungusSetB(clay,x+1,cy+y,z,logblock);
						worldgenSurroundWithLeafes(wgen,x+1,cy+y,z,leafes);
						if(rngValM(4) == 0){
							chungusSetB(clay,x+2,cy+y,z,logblock);
							worldgenSurroundWithLeafes(wgen,x+2,cy+y,z,leafes);
						}
					break;

					case 2:
						chungusSetB(clay,x-1,cy+y,z,logblock);
						worldgenSurroundWithLeafes(wgen,x-1,cy+y,z,leafes);
						if(rngValM(4) == 0){
							chungusSetB(clay,x-2,cy+y,z,logblock);
							worldgenSurroundWithLeafes(wgen,x-2,cy+y,z,leafes);
						}
					break;

					case 3:
						chungusSetB(clay,x,cy+y,z+1,logblock);
						worldgenSurroundWithLeafes(wgen,x,cy+y,z+1,leafes);
						if(rngValM(4) == 0){
							chungusSetB(clay,x,cy+y,z+2,logblock);
							worldgenSurroundWithLeafes(wgen,x,cy+y,z+2,leafes);
						}
					break;

					case 4:
						chungusSetB(clay,x,cy+y,z-1,logblock);
						worldgenSurroundWithLeafes(wgen,x,cy+y,z-1,leafes);
						if(rngValM(4) == 0){
							chungusSetB(clay,x,cy+y,z-2,logblock);
							worldgenSurroundWithLeafes(wgen,x,cy+y,z-2,leafes);
						}
					break;
				}
			}
		}
	}
	worldgenRoots(wgen,x,y-1,z);
}

void worldgenBigOak(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	int size       = rngValMM(18,24);
	int sparseness = rngValMM(2,4);
	int lsize      = 8;
	int leafes     = 11;
	int logblock   = 10;
	if(rngValM(16) == 0){leafes   = 19;} // We Sakura now
	if(rngValM(16) == 0){logblock = 20;} // We Birch now

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
					chungusSetB(clay,cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < -2){
			chungusSetB(clay,x  ,cy+y,z  ,8);
			chungusSetB(clay,x+1,cy+y,z  ,8);
			chungusSetB(clay,x  ,cy+y,z+1,8);
			chungusSetB(clay,x+1,cy+y,z+1,8);
		}else if(cy < size-2){
			chungusSetB(clay,x  ,cy+y,z  ,logblock);
			chungusSetB(clay,x+1,cy+y,z  ,logblock);
			chungusSetB(clay,x  ,cy+y,z+1,logblock);
			chungusSetB(clay,x+1,cy+y,z+1,logblock);
		}
	}
	worldgenBigRoots(wgen,x,y-5,z);
}
