#include "labyrinth.h"

#include "../game/animal.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <string.h>
#include <stdio.h>

/*
static int labMapGetCell(u64 x, u64 y, u64 z){
	const uint layer = 5 - abs(4 - (((int)y >> 8) - 8));
	u64 seed = (x >> 4) | (y >> 4) | (z >> 4);
	seed = ((seed * 1103515245)) + 12345;
	//fprintf(stderr,"%llu %llu %llu = %llx\n",x,y,z,seed);
	return ((seed >> 4) & 0x7) < layer;
}*/

static void labMapRandomize(const worldgen *wgen, u8 labMap[16][16][16]){
	const uint labLayer = abs(4 - (wgen->layer - 8));
	for(int cx = 15; cx >= 0; cx--){
		for(int cy = 15; cy >= 0; cy--){
			for(int cz = 15; cz >= 0; cz--){
				labMap[cx][cy][cz] = (rngValM((5-labLayer) << 1)) == 0;
				//labMap[cx][cy][cz] = labMapGetCell(wgen->gx | (cx << 4),wgen->gy | (cy << 4), wgen->gz | (cz << 4));
			}
		}
	}
}

static void labMapStep(u8 labMap[16][16][16]){
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

static void labPavillon(const worldgen *wgen,const ivec pos, u8 roofBlock, u8 topRoofBlock){
	chungus *clay = wgen->clay;
	const int bx = (pos.x<<4)+3;
	const int by = (pos.y<<4)+6;
	const int bz = (pos.z<<4)+3;

	chungusBox(clay,bx  ,by+7,bz  ,10,1,10,roofBlock);
	chungusBox(clay,bx+1,by+8,bz+1, 8,1, 8,topRoofBlock);
	chungusBox(clay,bx+1,by+7,bz+1, 8,1, 8,0);
	chungusBox(clay,bx-1,by+6,bz-1,12,1,12,roofBlock);
	chungusBox(clay,bx  ,by+6,bz  ,10,1,10,0);

	chungusBox(clay,bx  ,by+2,bz  ,1,5,1,I_Marble_Pillar);
	chungusBox(clay,bx+9,by+2,bz  ,1,5,1,I_Marble_Pillar);
	chungusBox(clay,bx  ,by+2,bz+9,1,5,1,I_Marble_Pillar);
	chungusBox(clay,bx+9,by+2,bz+9,1,5,1,I_Marble_Pillar);
}

static void labPlatform(const worldgen *wgen,const ivec pos, uint pType){
	chungus *clay = wgen->clay;
	const int bx = (pos.x<<4)+3;
	const int by = (pos.y<<4)+6;
	const int bz = (pos.z<<4)+3;

	chungusBox(clay,bx+1,by-1,bz+1, 8,1, 8,I_Marble_Block);
	chungusBox(clay,bx  ,by  ,bz  ,10,1,10,I_Marble_Blocks);

	chungusBox(clay,bx  ,by+1,bz  ,10,1, 1,I_Marble_Block);
	chungusBox(clay,bx  ,by+1,bz+9,10,1, 1,I_Marble_Block);

	chungusBox(clay,bx  ,by+1,bz  , 1,1,10,I_Marble_Block);
	chungusBox(clay,bx+9,by+1,bz  , 1,1,10,I_Marble_Block);

	switch(pType){
	default:
	case 0:
		break;
	case 1:
		labPavillon(wgen,pos,I_Marble_Block,I_Marble_Block);
		break;
	case 2:
		labPavillon(wgen,pos,I_Obsidian,I_Obsidian);
		break;
	case 3:
		labPavillon(wgen,pos,I_Crystal,I_Crystal);
		break;
	case 4:
		labPavillon(wgen,pos,I_Marble_Block,0);
		break;
	}
}

void labBridgeX(const worldgen *wgen,const ivec pos, int style, int size){
	chungus *clay = wgen->clay;
	const int bx = (pos.x<<4)+3;
	const int by = (pos.y<<4)+6;
	      int bz = (pos.z<<4)+3;

	if(style == 0){
		chungusBox(clay,bx+ 9,by+1,bz+1,size  ,2,8,0);
		chungusBox(clay,bx+10,by  ,bz+1,size-2,1,8,I_Marble_Blocks);
		chungusBox(clay,bx+10,by+1,bz  ,size-2,1,1,I_Marble_Block);
		chungusBox(clay,bx+10,by+1,bz+9,size-2,1,1,I_Marble_Block);
		return;
	}
	if(style == 2){ bz += 3; }
	if(style == 3){ bz -= 3; }
	chungusBox(clay,bx+10,by  ,bz+3,size-2,1,4,I_Marble_Blocks);
	chungusBox(clay,bx+ 9,by+1,bz+4,size  ,2,2,0);
	chungusBox(clay,bx+10,by+1,bz+3,size-2,1,1,I_Marble_Block);
	chungusBox(clay,bx+10,by+1,bz+6,size-2,1,1,I_Marble_Block);
}

void labBridgeZ(const worldgen *wgen,const ivec pos, int style, int size){
	chungus *clay = wgen->clay;
	      int bx = (pos.x<<4)+3;
	const int by = (pos.y<<4)+6;
	const int bz = (pos.z<<4)+3;

	if(style == 0){
		chungusBox(clay,bx+1,by+1,bz+ 9,8,2,size  ,0);
		chungusBox(clay,bx+1,by  ,bz+10,8,1,size-2,I_Marble_Blocks);
		chungusBox(clay,bx  ,by+1,bz+10,1,1,size-2,I_Marble_Block);
		chungusBox(clay,bx+9,by+1,bz+10,1,1,size-2,I_Marble_Block);
		return;
	}
	if(style == 2){ bx += 3; }
	if(style == 3){ bx -= 3; }
	chungusBox(clay,bx+3,by  ,bz+10,4,1,size-2,I_Marble_Blocks);
	chungusBox(clay,bx+4,by+1,bz+ 9,2,2,size  ,0);
	chungusBox(clay,bx+3,by+1,bz+10,1,1,size-2,I_Marble_Block);
	chungusBox(clay,bx+6,by+1,bz+10,1,1,size-2,I_Marble_Block);
}

static void labStairsXGT(const worldgen *wgen, const ivec pos, int m){
	chungus *clay = wgen->clay;
	const int bx = (pos.x<<4)+8;
	const int by = (pos.y<<4)+6;
	const int bz = (pos.z<<4)+9;

	chungusSetB(clay,bx+4,by+1,bz+1,0);
	chungusSetB(clay,bx+4,by+1,bz+2,0);
	for(int i=0;i<16*m;i++){
		const uint cx = bx+i;
		const uint cy = by+i/m;
		chungusSetB(clay,cx,cy-1,bz+1,I_Marble_Block);
		chungusSetB(clay,cx,cy-1,bz+2,I_Marble_Block);

		chungusSetB(clay,cx,cy  ,bz  ,I_Marble_Block);
		chungusSetB(clay,cx,cy  ,bz+1,I_Marble_Blocks);
		chungusSetB(clay,cx,cy  ,bz+2,I_Marble_Blocks);
		chungusSetB(clay,cx,cy  ,bz+3,I_Marble_Block);

		chungusSetB(clay,cx,cy+1,bz  ,I_Marble_Block);
		chungusSetB(clay,cx,cy+1,bz+3,I_Marble_Block);

		chungusSetB(clay,cx,cy+1,bz+1,0);
		chungusSetB(clay,cx,cy+1,bz+2,0);
		chungusSetB(clay,cx,cy+2,bz+1,0);
		chungusSetB(clay,cx,cy+2,bz+2,0);
		chungusSetB(clay,cx,cy+3,bz+1,0);
		chungusSetB(clay,cx,cy+3,bz+2,0);
		chungusSetB(clay,cx,cy+4,bz+1,0);
		chungusSetB(clay,cx,cy+4,bz+2,0);
	}
}

static void labStairsZGT(const worldgen *wgen, const ivec pos, int m){
	chungus *clay = wgen->clay;
	const int bx = (pos.x<<4)+4;
	const int by = (pos.y<<4)+6;
	const int bz = (pos.z<<4)+8;

	chungusSetB(clay,bx+1,by+1,bz+4,0);
	chungusSetB(clay,bx+2,by+1,bz+4,0);
	for(int i=0;i<16*m;i++){
		const uint cy = by+i/m;
		const uint cz = bz+i;
		chungusSetB(clay,bx+1,cy-1,cz,I_Marble_Block);
		chungusSetB(clay,bx+2,cy-1,cz,I_Marble_Block);

		chungusSetB(clay,bx  ,cy  ,cz,I_Marble_Block);
		chungusSetB(clay,bx+1,cy  ,cz,I_Marble_Blocks);
		chungusSetB(clay,bx+2,cy  ,cz,I_Marble_Blocks);
		chungusSetB(clay,bx+3,cy  ,cz,I_Marble_Block);

		chungusSetB(clay,bx  ,cy+1,cz,I_Marble_Block);
		chungusSetB(clay,bx+3,cy+1,cz,I_Marble_Block);

		chungusSetB(clay,bx+1,cy+1,cz,0);
		chungusSetB(clay,bx+2,cy+1,cz,0);
		chungusSetB(clay,bx+1,cy+2,cz,0);
		chungusSetB(clay,bx+2,cy+2,cz,0);
		chungusSetB(clay,bx+1,cy+3,cz,0);
		chungusSetB(clay,bx+2,cy+3,cz,0);
		chungusSetB(clay,bx+1,cy+4,cz,0);
		chungusSetB(clay,bx+2,cy+4,cz,0);
	}
}

void worldgenLabyrinth(worldgen *wgen){
	static u8 labMap[16][16][16];
	labMapRandomize(wgen,labMap);
	for(int i=0;i<4;i++){
		labMapStep(labMap);
	}
	for(int cx = 15; cx >= 0; cx--){
		for(int cy = 15; cy >= 0; cy--){
			for(int cz = 15; cz >= 0; cz--){
				if(!labMap[cx][cy][cz]){continue;}
				const ivec pos = ivecNew(cx,cy,cz);

				labPlatform(wgen, pos, rngValM(32));
				if((cx < 15) && labMap[cx+1][cy][cz]){
					labBridgeX(wgen,pos,rngValM(4),8);
				}
				if((cx < 14) && labMap[cx+2][cy][cz] && !labMap[cx+1][cy][cz]){
					labBridgeX(wgen,pos,rngValM(4),8+16);
				}
				if((cz < 15) && labMap[cx][cy][cz+1]){
					labBridgeZ(wgen,pos,rngValM(4),8);
				}
				if((cz < 14) && labMap[cx][cy][cz+2] && !labMap[cx][cy][cz+1]){
					labBridgeZ(wgen,pos,rngValM(4),8+16);
				}

				if((cx < 15) && (cy < 15) && labMap[cx+1][cy+1][cz]){
					labStairsXGT(wgen,pos,1);
				}else if((cx < 14) && (cy < 15) && labMap[cx+2][cy+1][cz] && !labMap[cx+1][cy+1][cz] && !labMap[cx+1][cy][cz]){
					labStairsXGT(wgen,pos,2);
				}

				if((cz < 15) && (cy < 15) && labMap[cx][cy+1][cz+1]){
					labStairsZGT(wgen,pos,1);
				}else if((cz < 14) && (cy < 15) && labMap[cx][cy+1][cz+2] && !labMap[cx][cy+1][cz+1] && !labMap[cx][cy][cz+1]){
					labStairsZGT(wgen,pos,2);
				}

				if(rngValM(32) == 0){
					animalNew(vecNew(wgen->gx+(cx<<4)+6,wgen->gy+(cx<<4)+9,wgen->gz+(cx<<4)+6),2);
				}
			}
		}
	}

}
