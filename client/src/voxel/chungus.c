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
int chungusCount=0;
chungus *chungusFirstFree = NULL;

int chungusGetActiveCount(){
	return chungusCount;
}
void chungusSetActiveCount(int i){
	chungusCount = i;
}
chungus *chungusGetActive(int i){
	return &chungusList[i];
}

int chunkInFrustum(float x,float y,float z,float xoff,float yoff, float zoff){
	x = x * CHUNK_SIZE + xoff;
	y = y * CHUNK_SIZE + yoff;
	z = z * CHUNK_SIZE + zoff;
	return CubeInFrustum(x,y,z,CHUNK_SIZE);
}

float chunkDistance(const character *cam, float x, float y,float z){
	float xdiff = (float)x-cam->x;
	float ydiff = (float)y-cam->y;
	float zdiff = (float)z-cam->z;
	return sqrtf((xdiff*xdiff)+(ydiff*ydiff)+(zdiff*zdiff));
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

void chungusQueueDraws(chungus *c, character *cam, queueEntry *drawQueue,int *drawQueueLen){
	for(int x=0;x<16;x++){
		const int cx = ((x*CHUNK_SIZE)+(CHUNK_SIZE/2))+c->x;
		for(int y=0;y<16;y++){
			const int cy = ((y*CHUNK_SIZE)+(CHUNK_SIZE/2))+c->y;
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] == NULL){continue;}
				if(!chunkInFrustum(x,y,z,c->x,c->y,c->z)){continue;}
				const int cz = ((z*CHUNK_SIZE)+(CHUNK_SIZE/2))+c->z;
				const float d = chunkDistance(cam,cx,cy,cz);
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

uint8_t chungusGetB(chungus *c, int x,int y,int z){
	chunk *chnk;
	if(((x|y|z)>>4)&(~0xF)){return 0;}
	chnk = c->chunks[x>>4][y>>4][z>>4];
	if(chnk == NULL){return 0;}
	return chnk->data[x&0xF][y&0xF][z&0xF];
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
}
