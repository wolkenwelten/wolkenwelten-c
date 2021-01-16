#include "vegetation.h"

#include "../mods/api_v1.h"
#include "../network/messages.h"

static void vegSetB(u16 x, u16 y, u16 z, u8 b){
	if(!isClient){msgMineBlock(x,y,z,b,2);}
	worldSetB(x,y,z,b);
}

void vegShrub(int x,int y,int z){
	int leaveBlock = 21;
	vegSetB(x,y,z,I_Roots);
	vegSetB(x,y+1,z,leaveBlock);
	if(rngValM(2) == 0){
		vegSetB(x,y+2,z,leaveBlock);
	}
}

void vegRoots(int x,int y,int z){
	int size = rngValMM(4,12);
	for(int cy = 0;cy > -size;--cy){
		u8 b = worldGetB(x,y+cy,z);

		switch(b){
		case 0:
		case 1:
		case 2:
		case 6:
			vegSetB(x,y+cy,z,I_Roots);
			break;

		default:
			return;
		}
		b = worldGetB(x-1,y+cy,z);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			vegSetB(x-1,y+cy,z,I_Roots);
		}
		b = worldGetB(x+1,y+cy,z);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			vegSetB(x+1,y+cy,z,I_Roots);
		}
		b = worldGetB(x,y+cy,z-1);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			vegSetB(x,y+cy,z-1,I_Roots);
		}
		b = worldGetB(x,y+cy,z+1);
		if(((b == 1) || (b == 2)) && (rngValM(2)==0)){
			vegSetB(x,y+cy,z+1,I_Roots);
		}
	}
}

void vegBigRoots(int x,int y,int z){
	for(int cx = 0;cx < 2;cx++){
		for(int cz = 0;cz < 2;cz++){
			vegRoots(cx+x,y,cz+z);
		}
	}
}

void vegDeadTree(int x,int y,int z){
	int size       = rngValMM(12,16);
	for(int cy = 0;cy < size;cy++){
		vegSetB(x,cy+y,z,5);
	}
	vegRoots(x,y-1,z);
}

void vegBigDeadTree(int x,int y,int z){
	int size       = rngValMM(20,34);
	for(int cy = -5;cy < size;cy++){
		if(cy < -2){
			vegSetB(x  ,cy+y,z  ,8);
			vegSetB(x+1,cy+y,z  ,8);
			vegSetB(x  ,cy+y,z+1,8);
			vegSetB(x+1,cy+y,z+1,8);
		}else{
			vegSetB(x  ,cy+y,z  ,5);
			vegSetB(x+1,cy+y,z  ,5);
			vegSetB(x  ,cy+y,z+1,5);
			vegSetB(x+1,cy+y,z+1,5);
		}
	}
	vegBigRoots(x,y-5,z);
}

void vegSpruce(int x,int y,int z){
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
					vegSetB(cx+x,cy+y,cz+z,6);
				}
			}
		}
		vegSetB(x,cy+y,z,5);
	}
	vegSetB(x,size+y  ,z,6);
	vegSetB(x,size+y+1,z,6);
	vegRoots(x,y-1,z);
}

void vegBigSpruce(int x,int y,int z){
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
					vegSetB(cx+x,cy+y,cz+z,6);
				}
			}
		}
		if(cy < -2){
			vegSetB(x  ,cy+y,z  ,8);
			vegSetB(x+1,cy+y,z  ,8);
			vegSetB(x  ,cy+y,z+1,8);
			vegSetB(x+1,cy+y,z+1,8);
		}else{
			vegSetB(x  ,cy+y,z  ,5);
			vegSetB(x+1,cy+y,z  ,5);
			vegSetB(x  ,cy+y,z+1,5);
			vegSetB(x+1,cy+y,z+1,5);
		}
	}
	for(int cx=0;cx<2;cx++){
		for(int cy=0;cy<2;cy++){
			for(int cz=0;cz<2;cz++){
				vegSetB(x+cx,size+y+cy,z+cz,6);
			}
		}
	}
	vegBigRoots(x,y-5,z);
}

void vegSurroundWithLeafes(int x, int y, int z, u8 leafB){
	for(int cx=-1;cx<=1;cx++){
		for(int cy=0;cy<=1;cy++){
			for(int cz=-1;cz<=1;cz++){
				if(worldGetB(x+cx,y+cy,z+cz) != 0){continue;}
				vegSetB(x+cx,y+cy,z+cz,leafB);
			}
		}
	}
}

void vegOak(int x,int y,int z){
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
						vegSetB(cx+x,cy+y,cz+z,leafes);
						continue;
					}
					if((cx == -lsize) && (cz == -lsize  )){continue;}
					if((cx == -lsize) && (cz ==  lsize  )){continue;}
					if((cx ==  lsize) && (cz == -lsize  )){continue;}
					if((cx ==  lsize) && (cz ==  lsize  )){continue;}
					if((rngValM(sparseness)) == 0)        {continue;}
					vegSetB(cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < size-2){
			vegSetB(x,cy+y,z,logblock);
			if(cy > 3){
				r = rngValM(8);
				switch(r){
					case 1:
						vegSetB(x+1,cy+y,z,logblock);
						vegSurroundWithLeafes(x+1,cy+y,z,leafes);
						if(rngValM(4) == 0){
							vegSetB(x+2,cy+y,z,logblock);
							vegSurroundWithLeafes(x+2,cy+y,z,leafes);
						}
					break;

					case 2:
						vegSetB(x-1,cy+y,z,logblock);
						vegSurroundWithLeafes(x-1,cy+y,z,leafes);
						if(rngValM(4) == 0){
							vegSetB(x-2,cy+y,z,logblock);
							vegSurroundWithLeafes(x-2,cy+y,z,leafes);
						}
					break;

					case 3:
						vegSetB(x,cy+y,z+1,logblock);
						vegSurroundWithLeafes(x,cy+y,z+1,leafes);
						if(rngValM(4) == 0){
							vegSetB(x,cy+y,z+2,logblock);
							vegSurroundWithLeafes(x,cy+y,z+2,leafes);
						}
					break;

					case 4:
						vegSetB(x,cy+y,z-1,logblock);
						vegSurroundWithLeafes(x,cy+y,z-1,leafes);
						if(rngValM(4) == 0){
							vegSetB(x,cy+y,z-2,logblock);
							vegSurroundWithLeafes(x,cy+y,z-2,leafes);
						}
					break;
				}
			}
		}
	}
	vegRoots(x,y-1,z);
}

void vegBigOak(int x,int y,int z){
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
					vegSetB(cx+x,cy+y,cz+z,leafes);
				}
			}
		}
		if(cy < -2){
			vegSetB(x  ,cy+y,z  ,8);
			vegSetB(x+1,cy+y,z  ,8);
			vegSetB(x  ,cy+y,z+1,8);
			vegSetB(x+1,cy+y,z+1,8);
		}else if(cy < size-2){
			vegSetB(x  ,cy+y,z  ,logblock);
			vegSetB(x+1,cy+y,z  ,logblock);
			vegSetB(x  ,cy+y,z+1,logblock);
			vegSetB(x+1,cy+y,z+1,logblock);
		}
	}
	vegBigRoots(x,y-5,z);
}
