#include "chungus.h"

#include "../game/entity.h"
#include "../game/blockType.h"
#include "../gfx/frustum.h"
#include "../gfx/texture.h"
#include "../../../common/src/misc/misc.h"
#include "../voxel/chunk.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

chungus chungusList[1<<9];
uint chungusCount=0;
chungus *chungusFirstFree = NULL;

uint chungusGetActiveCount(){
	return chungusCount;
}
void chungusSetActiveCount(uint i){
	chungusCount = i;
}
chungus *chungusGetActive(uint i){
	return &chungusList[i];
}

int chunkInFrustum(const vec pos, const vec off){
	return CubeInFrustum(vecAdd(off,vecMulS(pos,CHUNK_SIZE)),CHUNK_SIZE);
}

float chunkDistance(const vec cam, const vec pos){
	return vecMag(vecSub(pos,cam));
}

chungus *chungusNew(int x, int y, int z){
	chungus *c = NULL;
	if(chungusFirstFree == NULL){
		if(chungusCount >= (int)(sizeof(chungusList) / sizeof(chungus))-1){
			fprintf(stderr,"client chungusList Overflow!\n");
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
	c->loaded = 0;

	memset(c->chunks,0,16*16*16*sizeof(chunk *));
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

void chungusQueueDraws(chungus *c,const character *cam, queueEntry *drawQueue,int *drawQueueLen){
	const vec coff = vecNew(c->x,c->y,c->z);
	for(int x=0;x<16;x++){
		const int cx = x*CHUNK_SIZE+CHUNK_SIZE/2+c->x;
		for(int y=0;y<16;y++){
			const int cy = y*CHUNK_SIZE+CHUNK_SIZE/2+c->y;
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] == NULL){continue;}
				if(!chunkInFrustum(vecNew(x,y,z),coff)){continue;}
				const int cz = z*CHUNK_SIZE+CHUNK_SIZE/2+c->z;
				const float d = chunkDistance(cam->pos,vecNew(cx,cy,cz));
				if(d > CHUNK_RENDER_DISTANCE){continue;}
				drawQueue[*drawQueueLen].distance = d;
				drawQueue[*drawQueueLen].chnk = c->chunks[x][y][z];

				*drawQueueLen = *drawQueueLen+1;
			}
		}
	}
}

chunk *chungusGetChunk(chungus *c, int x,int y,int z){
	if(((x|y|z)>>4)&(~0xF)){return NULL;}
	return c->chunks[x>>4][y>>4][z>>4];
}

chunk *chungusGetChunkOrNew(chungus *c, int x, int y, int z){
	chunk *chnk;
	int cx = (x >> 4) & 0xF;
	int cy = (y >> 4) & 0xF;
	int cz = (z >> 4) & 0xF;
	chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew(c->x+(cx << 4),c->y+(cy << 4),c->z+(cz << 4));
	}
	return chnk;
}

u8 chungusGetB(chungus *c, int x,int y,int z){
	chunk *chnk;
	if(((x|y|z)>>4)&(~0xF)){return 0;}
	chnk = c->chunks[x>>4][y>>4][z>>4];
	if(chnk == NULL){return 0;}
	return chnk->data[x&0xF][y&0xF][z&0xF];
}

void chungusSetB(chungus *c, int x,int y,int z,u8 block){
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
}

void chungusBoxF(chungus *c, int x,int y,int z, int w,int h,int d,u8 block){
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

void chungusBox(chungus *c, int x,int y,int z, int w,int h,int d,u8 block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				chungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}
