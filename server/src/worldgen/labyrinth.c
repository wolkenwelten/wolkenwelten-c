#include "labyrinth.h"

#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

#include <string.h>

void worldgenLabyrinth(worldgen *wgen, int labLayer){
	chungus *clay = wgen->clay;
	unsigned char labMap[16][16][16];
	int b = 12;
	int pb = 14;
	int fb = 15;
	memset(labMap,0,sizeof(labMap));
	// First lets randomly set some coordinates
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

	// Now free some when there are too many, especially if immediatly above/below
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

				// First draw the default platform
				chungusBox(clay,bx+1,by-1,bz+1, 8,1, 8,b);
				chungusBox(clay,bx  ,by  ,bz  ,10,1,10,fb);

				chungusBox(clay,bx  ,by+1,bz  ,10,1, 1,b);
				chungusBox(clay,bx  ,by+1,bz+9,10,1, 1,b);

				chungusBox(clay,bx  ,by+1,bz  , 1,1,10,b);
				chungusBox(clay,bx+9,by+1,bz  , 1,1,10,b);

				// Change the base platform up a bit
				switch(rngValM(32)){
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

						chungusBox(clay,bx+3,by+1,bz+3,4,2,4,13);
					break;

					case 2:
						chungusBox(clay,bx  ,by+2,bz  ,1,5,1,pb);
						chungusBox(clay,bx+9,by+2,bz  ,1,5,1,pb);
						chungusBox(clay,bx  ,by+2,bz+9,1,5,1,pb);
						chungusBox(clay,bx+9,by+2,bz+9,1,5,1,pb);

						chungusBox(clay,bx  ,by+7,bz  ,10,1,10,4);
						chungusBox(clay,bx+1,by+7,bz+1, 8,1, 8,0);
						chungusBox(clay,bx+1,by+8,bz+1, 8,1, 8,4);
					break;

					case 3:
						chungusBox(clay,bx+4,by  ,bz+4,2,5,2,18);
					break;

					case 4:
						chungusBox(clay,bx+4,by  ,bz+4,2,5,2,13);
					break;

					case 5:
						chungusBox(clay,bx+4,by  ,bz+4,2,5,2,4);
					break;

					case 6:
						chungusBox(clay,bx  ,by+2,bz  ,1,5,1,pb);
						chungusBox(clay,bx+9,by+2,bz  ,1,5,1,pb);
						chungusBox(clay,bx  ,by+2,bz+9,1,5,1,pb);
						chungusBox(clay,bx+9,by+2,bz+9,1,5,1,pb);

						chungusBox(clay,bx  ,by+7,bz  ,10,1,10,18);
						chungusBox(clay,bx+1,by+7,bz+1, 8,1, 8,0);
						chungusBox(clay,bx+1,by+8,bz+1, 8,1, 8,18);
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
