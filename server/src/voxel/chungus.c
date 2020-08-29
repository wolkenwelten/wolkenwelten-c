#include "chungus.h"

#include "../game/entity.h"
#include "../network/server.h"
#include "../worldgen/worldgen.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/game/blockType.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


chungus chungusList[1 << 12];
unsigned int chungusCount=0;
chungus *chungusFirstFree = NULL;

unsigned int chungusGetActiveCount(){
	return chungusCount;
}
void chungusSetActiveCount(unsigned int i){
	chungusCount = i;
}
chungus *chungusGetActive(unsigned int i){
	return &chungusList[i];
}

float chunkDistance(const entity *cam, float x, float y,float z){
	float xdiff = (float)x-cam->x;
	float ydiff = (float)y-cam->y;
	float zdiff = (float)z-cam->z;
	return sqrtf((xdiff*xdiff)+(ydiff*ydiff)+(zdiff*zdiff));
}

chungus *chungusNew(int x, int y, int z){
	chungus *c = NULL;
	if(chungusFirstFree == NULL){
		if(chungusCount >= (int)(sizeof(chungusList) / sizeof(chungus))-1){
			fprintf(stderr,"server chungusList Overflow!\n");
			return NULL;
		}
		c = &chungusList[chungusCount++];
	}else{
		c = chungusFirstFree;
		chungusFirstFree = c->nextFree;
	}

	c->x = x;
	c->y = y;
	c->z = z;
	c->nextFree = NULL;
	c->spawnx = c->spawny = c->spawnz = -1;
	c->clientsSubscribed  = 0;
	c->clientsUpdated     = 0;
	memset(c->chunks,0,16*16*16*sizeof(chunk *));
	worldgen *wgen = worldgenNew(c);
	worldgenGenerate(wgen);
	worldgenFree(wgen);

	return c;
}

void chungusFree(chungus *c){
	if(c == NULL){return;}
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				chunkFree(c->chunks[x][y][z]);
				c->chunks[x][y][z] = NULL;
			}
		}
	}
	c->nextFree = chungusFirstFree;
	chungusFirstFree = c;
}

chunk *chungusGetChunk(chungus *c, int x,int y,int z){
	if(((x|y|z)>>4)&(~0xF)){return NULL;}
	return c->chunks[x>>4][y>>4][z>>4];
}

uint8_t chungusGetB(chungus *c, int x,int y,int z){
	chunk *chnk;
	if(((x|y|z)>>4)&(~0xF)){return 0;}
	chnk = c->chunks[x>>4][y>>4][z>>4];
	if(chnk == NULL){return 0;}
	return chnk->data[x&0xF][y&0xF][z&0xF];
}

int chungusGetHighestP(chungus *c, int x, int *retY, int z){
	uint8_t b;
	chunk *chnk;

	if((x|z)&(~0xFF)){return 0;}
	int cx = x >> 4;
	int cz = z >> 4;
	x &= 0xF;
	z &= 0xF;

	for(int cy=15;cy >= 0;cy--){
		chnk = c->chunks[cx][cy][cz];
		if(chnk == NULL){continue;}
		for(int y=CHUNK_SIZE-1;y>=0;y--){
			b = chnk->data[x&0xF][y&0xF][z&0xF];
			if(b != 0){
				*retY = (cy*CHUNK_SIZE)+y;
				return 1;
			}
		}
	}
	return 0;
}

void chungusSetB(chungus *c, int x,int y,int z,uint8_t block){
	chunk *chnk;
	if((x|y|z)&(~0xFF)){return;}
	int cx = (x >> 4) & 0xF;
	int cy = (y >> 4) & 0xF;
	int cz = (z >> 4) & 0xF;
	chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew(c->x+(cx << 4),c->y+(cy << 4),c->z+(cz << 4));
	}
	chunkSetB(chnk,x,y,z,block);
	c->clientsUpdated = 0;
}

void chungusBoxF(chungus *c, int x,int y,int z, int w,int h,int d,uint8_t block){
	const int gx = (x+w)>>4;
	const int gy = (y+h)>>4;
	const int gz = (z+d)>>4;
	if((x|y|z)&(~0xFF)){return;}
	if(((x+w)|(y+h)|(z+d))&(~0xFF)){return;}
	int sx = x&0xF;
	int sw = CHUNK_SIZE;
	for(int cx=x>>4;cx<=gx;cx++){
		int sy = y&0xF;
		int sh = CHUNK_SIZE;
		if(cx == gx){
			sw = (x+w)&0xF;
		}
		for(int cy=y>>4;cy<=gy;cy++){
			int sz = z&0xF;
			int sd = CHUNK_SIZE;
			if(cy == gy){
				sh = (y+h)&0xF;
			}
			for(int cz=z>>4;cz<=gz;cz++){
				chunk *chnk = c->chunks[cx&0xF][cy&0xF][cz&0xF];
				if(chnk == NULL){
					c->chunks[cx&0xF][cy&0xF][cz&0xF] = chnk = chunkNew(c->x+(cx<<4),c->y+(cy<<4),c->z+(cz<<4));
				}
				if(cz == gz){
					sd = (z+d)&0xF;
				}
				chunkBox(chnk,sx,sy,sz,sw,sh,sd,block);
				sz = 0;
			}
			sy = 0;
		}
		sx = 0;
	}
}

void chungusBox(chungus *c, int x,int y,int z, int w,int h,int d,uint8_t block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				chungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
	c->clientsUpdated = 0;
}

void chungusRoughBox(chungus *c, int x,int y,int z, int w,int h,int d,uint8_t block){
	int dx = x+w-1;
	int dy = y+h-1;
	int dz = z+d-1;

	for(int cx=x;cx<=dx;cx++){
		for(int cy=y;cy<=dy;cy++){
			for(int cz=z;cz<=dz;cz++){
				if((cx == x) || (cx == dx) || (cy == y) || (cy == dy) || (cz == z) || (cz == dz)){
					if((rngValR()&0x100) == 0){continue;}
				}
				chungusSetB(c,cx,cy,cz,block);
			}
		}
	}
	c->clientsUpdated = 0;
}

void chungusRandomBox(chungus *c, int x,int y,int z, int w,int h,int d,uint8_t block,int chance){
	int dx = x+w;
	int dy = y+h;
	int dz = z+d;

	for(int cx=x;cx<dx;cx++){
		for(int cy=y;cy<dy;cy++){
			for(int cz=z;cz<dz;cz++){
				if((rngValR()&chance) != 0){continue;}
				chungusSetB(c,cx,cy,cz,block);
			}
		}
	}
	c->clientsUpdated = 0;
}

void chungusFill(chungus *c, int x,int y,int z,uint8_t b){
	chunk *chnk;
	int cx = (x / CHUNK_SIZE) & 0xF;
	int cy = (y / CHUNK_SIZE) & 0xF;
	int cz = (z / CHUNK_SIZE) & 0xF;
	chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew(c->x+cx*CHUNK_SIZE,c->y+cy*CHUNK_SIZE,c->z+cz*CHUNK_SIZE);
	}
	chunkFill(chnk,b);
	c->clientsUpdated = 0;
}

void chungusSubscribePlayer(chungus *c, int p){
	if(c == NULL){return;}
	c->clientsSubscribed |= 1 << p;
}
int chungusUnsubscribePlayer(chungus *c, int p){
	if(c == NULL)                         {return 0;}
	if(!(c->clientsSubscribed & (1 << p))){return 1;}
	
	uint32_t mask = ~(1 << p);
	c->clientsSubscribed &= mask;
	c->clientsUpdated    &= mask;
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] != NULL){
					c->chunks[x][y][z]->clientsUpdated &= mask;
				}
			}
		}
	}
	return 0;
}
int chungusIsSubscribed(chungus *c, int p){
	return c->clientsSubscribed & (1 << p);
}
int chungusIsUpdated(chungus *c, int p){
	if(c == NULL){return 1;}
	return c->clientsUpdated & (1 << p);
}
void chungusSetUpdated(chungus *c, int p){
	if( c == NULL){return;}
	c->clientsUpdated |= 1 << p;
}
int chungusUpdateClient(chungus *c, int p){
	if(c == NULL){return 1;}
	if(!chungusIsSubscribed(c,p)){return 1;}
	if( chungusIsUpdated(c,p)){return 0;}
	addChungusToQueue(p,c->x,c->y,c->z);
	chungusSetUpdated(c,p);
	return 0;

}
