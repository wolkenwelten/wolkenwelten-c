#include "bigchungus.h"

#include "../network/server.h"
#include "../game/entity.h"
#include "../../../common/src/game/blockType.h"
#include "../game/blockMining.h"
#include "../misc/noise.h"
#include "../misc/options.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bigchungus world;

float chungusRoughDistance(character *cam, float x, float y,float z) {
	x = x * CHUNGUS_SIZE + CHUNGUS_SIZE/2;
	y = y * CHUNGUS_SIZE + CHUNGUS_SIZE/2;
	z = z * CHUNGUS_SIZE + CHUNGUS_SIZE/2;

	float xdiff = fabsf(x-cam->x);
	float ydiff = fabsf(y-cam->y);
	float zdiff = fabsf(z-cam->z);

	return sqrtf((xdiff*xdiff)+(ydiff*ydiff)+(zdiff*zdiff));
}

void bigchungusInit(bigchungus *c){
	memset(c->chungi,0,256*128*256*sizeof(chunk *));
	generateNoise(optionWorldSeed ^ 0x84407db3, c->vegetationConcentration);
	generateNoise(optionWorldSeed ^ 0xc2fb18f4, c->islandSizeModifier);
	generateNoise(optionWorldSeed ^ 0x1ab033cF, c->islandCountModifier);
	generateNoise(optionWorldSeed ^ 0xF79610E3, c->geoworldMap);
	generateNoise(optionWorldSeed ^ 0x36AC5890, c->heightModifier);
}

void bigchungusFree(bigchungus *c){
	for(int x=0;x<256;x++){
		for(int y=0;y<128;y++){
			for(int z=0;z<256;z++){
				chungusFree(c->chungi[x][y][z]);
			}
		}
	}
	memset(c->chungi,0,256*128*256*sizeof(chunk *));
}

chungus *bigchungusGetChungus(bigchungus *c, int x,int y,int z) {
	if((x|y|z)&(~0xFF)){return NULL;}
	chungus *chng = c->chungi[x&0xFF][y&0x7F][z&0xFF];
	if(chng == NULL){
		chng = c->chungi[x&0xFF][y&0x7F][z&0xFF] = chungusNew(x*CHUNGUS_SIZE,y*CHUNGUS_SIZE,z*CHUNGUS_SIZE);
	}
	return chng;
}

chunk *bigchungusGetChunk(bigchungus *c, int x, int y, int z){
	chungus *chng = bigchungusGetChungus(c,(x>>8)&0xFF,(y>>8)&0xFF,(z>>8)&0xFF);
	if(chng == NULL){return NULL;}
	chunk *chnk = chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
	return chnk;
}

uint8_t bigchungusGetB(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	chng = bigchungusGetChungus(c,x/CHUNGUS_SIZE,y/CHUNGUS_SIZE,z/CHUNGUS_SIZE);
	if(chng == NULL){ return 0; }
	return chungusGetB(chng,x%CHUNGUS_SIZE,y%CHUNGUS_SIZE,z%CHUNGUS_SIZE);
}

bool bigchungusGetHighestP(bigchungus *c, int x,int *retY, int z) {
	chungus *chng;

	if(x < 0){return 0;}
	if(z < 0){return 0;}
	int cx = x/CHUNGUS_SIZE;
	int cz = z/CHUNGUS_SIZE;
	if(cx >= 256){return 0;}
	if(cz >= 256){return 0;}
	x = x - (cx*CHUNGUS_SIZE);
	z = z - (cz*CHUNGUS_SIZE);

	for(int cy=127;cy >= 0;cy--){
		chng = c->chungi[cx][cy][cz];
		if(chng == NULL){
			chng = c->chungi[cx][cy][cz] = chungusNew(cx*CHUNGUS_SIZE,cy*CHUNGUS_SIZE,cz*CHUNGUS_SIZE);
		}
		int y;
		if(chungusGetHighestP(chng,x,&y,z)){
			*retY = y+(cy*CHUNGUS_SIZE);
			return true;
		}
	}
	return false;
}

bool bigchungusSetB(bigchungus *c, int x,int y,int z,uint8_t block){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng == NULL){
		c->chungi[cx][cy][cz] = chng = chungusNew(cx*CHUNGUS_SIZE,cy*CHUNGUS_SIZE,cz*CHUNGUS_SIZE);
		return true;
	}
	chungusSetB(chng,x%CHUNGUS_SIZE,y%CHUNGUS_SIZE,z%CHUNGUS_SIZE,block);
	return false;
}

void bigchungusBox(bigchungus *c, int x,int y,int z, int w,int h,int d,uint8_t block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				bigchungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void bigchungusBoxSphere(bigchungus *c, int x,int y,int z, int r, uint8_t block){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
		for(int cy=-r;cy<=r;cy++){
			for(int cz=-r;cz<=r;cz++){
				const int d = (cx*cx)+(cy*cy)+(cz*cz);
				if(d >= md){continue;}
				bigchungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void bigchungusBoxMine(bigchungus *c, int x,int y,int z, int w,int h,int d){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				uint8_t b = bigchungusGetB(c,cx+x,cy+y,cz+z);
				if(b==0){continue;}
				blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
				bigchungusSetB(c,cx+x,cy+y,cz+z,0);
			}
		}
	}
}

void bigchungusBoxMineSphere(bigchungus *c, int x,int y,int z, int r){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
		for(int cy=-r;cy<=r;cy++){
			for(int cz=-r;cz<=r;cz++){
				uint8_t b = bigchungusGetB(c,cx+x,cy+y,cz+z);
				if(b==0){continue;}
				const int d = (cx*cx)+(cy*cy)+(cz*cz);
				if(d >= md){continue;}
				blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
				bigchungusSetB(c,cx+x,cy+y,cz+z,0);
			}
		}
	}
}

int bigchungusTrySpawn(bigchungus *c, int sx, int sy, int sz){
	return( (bigchungusGetB(c,sx,sy  ,sz)!=0) &&
	        (bigchungusGetB(c,sx,sy+1,sz)==0) &&
	        (bigchungusGetB(c,sx,sy+2,sz)==0));
}

void bigchungusDetermineSpawn(bigchungus *c, int sx, int sy, int sz){
	sx &= ~0xFF;
	sy &= ~0xFF;
	sz &= ~0xFF;
	for(int step = CHUNGUS_SIZE; step >= 1;step/=2){
		for(int x = step/2;x<CHUNGUS_SIZE;x+=step){
			for(int y = step/2;y<CHUNGUS_SIZE;y+=step){
				for(int z = step/2;z<CHUNGUS_SIZE;z+=step){
					if(bigchungusTrySpawn(c,sx|x,sy|y,sz|z)){
						c->spawnx = sx|x;
						c->spawny = sy|y;
						c->spawnz = sz|z;
						return;
					}
				}
			}
		}
	}
}

void bigchungusGenSpawn(bigchungus *c){
	for(int x=127;x<=129;x++){
		for(int y=1;y<=3;y++){
			for(int z=127;z<=129;z++){
				if(c->chungi[x][y][z] == NULL){
					c->chungi[x][y][z] = chungusNew(x*CHUNGUS_SIZE,y*CHUNGUS_SIZE,z*CHUNGUS_SIZE);
				}
				if(c->chungi[x][y][z]->spawnx >= 0){
					c->spawnx = c->chungi[x][y][z]->spawnx + (CHUNGUS_SIZE * x);
					c->spawny = c->chungi[x][y][z]->spawny + (CHUNGUS_SIZE * y);
					c->spawnz = c->chungi[x][y][z]->spawnz + (CHUNGUS_SIZE * z);
				}
			}
		}
	}
	if(!bigchungusTrySpawn(c,c->spawnx,c->spawny,c->spawnz)){
		bigchungusDetermineSpawn(c,c->spawnx,c->spawny,c->spawnz);
	}
}

void bigchungusGetSpawnPos(bigchungus *c, int *x, int *y, int *z){
	if(!bigchungusTrySpawn(c,c->spawnx,c->spawny,c->spawnz)){
		bigchungusDetermineSpawn(c,c->spawnx,c->spawny,c->spawnz);
	}
	*x = c->spawnx;
	*y = c->spawny;
	*z = c->spawnz;
}

void bigchungusFreeFarChungi(bigchungus *c){
	int len = chungusGetActiveCount();
	for(int i=0;i<len;i++){
		chungus *chng = chungusGetActive(i);
		if(chng->nextFree != NULL){continue;}
		int x = (int)chng->x>>8;
		int y = (int)chng->y>>8;
		int z = (int)chng->z>>8;
		if((x >= 127) && (x <= 129) && (y >= 1) && (y <= 3) && (z >= 127) && (z <= 129)){continue;}

		for(int ii=0;ii<clientCount;++ii){
			if(chungusRoughDistance(clients[ii].c,x,y,z) < 768.f){ goto chungiDontFree; }
		}
		chungusFree(c->chungi[x][y][z]);
		c->chungi[x][y][z] = NULL;
		chungiDontFree:;
	}
}

void bigchungusUpdateClient(bigchungus *c, int p){
	character *chara = clients[p].c;
	int unsubs = 0;
	int cx = ((int)chara->x)>>8;
	int cy = ((int)chara->y)>>8;
	int cz = ((int)chara->z)>>8;
	
	if((cx >= 0) && (cx < 256) && (cy >= 0) && (cy < 128) && (cz >= 0) && (cz < 256)){
		chungusUpdateClient(world.chungi[cx][cy][cz],p);
	}
	for(int ix=0;ix < 12; ix++){
		for(int iy=0;iy < 12; iy++){
			for(int iz=0;iz < 12; iz++){
				
				int ox = ix >> 1;
				if(ix & 1){ox = -ox;}
				ox = cx+ox;
				if(ox <   0){goto xcontinue;}
				if(ox > 255){goto xcontinue;}
				
				int oy = iy >> 1;
				if(iy & 1){oy = -oy;}
				oy = cy+oy;
				if(oy <   0){goto ycontinue;}
				if(oy > 127){goto ycontinue;}
				
				int oz = iz >> 1;
				if(iz & 1){oz = -oz;}
				oz = cz+oz;
				if(oz <   0){continue;}
				if(oz > 255){continue;}
				
				chungus *chng = c->chungi[ox][oy][oz];
				if(chungusUpdateClient(chng,p)){if(++unsubs > 8){return;}}
			}
			ycontinue: (void)c;
		}
		xcontinue: (void)c;
	}
}

void bigchungusUnsubscribeClient(bigchungus *c, int p){
	character *chara = clients[p].c;
	int unsubs = 0;
	int cx,cy,cz;
	if(chara != NULL){
		cx = ((int)chara->x) >> 8;
		cy = ((int)chara->y) >> 8;
		cz = ((int)chara->z) >> 8;
	}else{
		cx = 128;
		cy =   4;
		cz = 128;
	}
	
	if((cx >= 0) && (cx < 256) && (cy >= 0) && (cy < 128) && (cz >= 0) && (cz < 256)){
		chungusUnsubscribePlayer(c->chungi[cx][cy][cz],p);
	}
	for(int ix=0;ix < 12; ix++){
		for(int iy=0;iy < 12; iy++){
			for(int iz=0;iz < 12; iz++){
				
				int ox = ix >> 1;
				if(ix & 1){ox = -ox;}
				ox = cx+ox;
				if(ox <   0){goto xcontinue;}
				if(ox > 255){goto xcontinue;}
				
				int oy = iy >> 1;
				if(iy & 1){oy = -oy;}
				oy = cy+oy;
				if(oy <   0){goto ycontinue;}
				if(oy > 127){goto ycontinue;}
				
				int oz = iz >> 1;
				if(iz & 1){oz = -oz;}
				oz = cz+oz;
				if(oz <   0){continue;}
				if(oz > 255){continue;}
				
				chungus *chng = c->chungi[ox][oy][oz];
				if(chungusUnsubscribePlayer(chng,p)){if(++unsubs > 8){return;}}
			}
			ycontinue: (void)c;
		}
		xcontinue: (void)c;
	}
}

void worldBox(int x, int y,int z, int w,int h,int d,uint8_t block){
	bigchungusBox(&world,x,y,z,w,h,d,block);
}
void worldBoxSphere(int x, int y,int z, int r,uint8_t block){
	bigchungusBoxSphere(&world,x,y,z,r,block);
}
uint8_t worldGetB(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z);
}
chungus* worldGetChungus(int x, int y, int z){
	return bigchungusGetChungus(&world,x,y,z);
}
chunk* worldGetChunk(int x, int y, int z){
	return bigchungusGetChunk(&world,x,y,z);
}
bool worldSetB(int x, int y, int z, uint8_t block){
	return bigchungusSetB(&world,x,y,z,block);
}
int checkCollision(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z) != 0;
}
