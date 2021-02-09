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

#include "chungus.h"

#include "../main.h"
#include "../sdl/sdl.h"
#include "../game/animal.h"
#include "../game/blockType.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/throwable.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#define CHUNGUS_COUNT (1<<12)

chungus *chungusList;
uint chungusCount=0;
chungus *chungusFirstFree = NULL;

void chungusInit(){
	chungusList = malloc(sizeof(chungus) * CHUNGUS_COUNT);
}

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

chungus *chungusNew(u8 x, u8 y, u8 z){
	chungus *c = NULL;
	if(chungusFirstFree == NULL){
		if(chungusCount+1 >= CHUNGUS_COUNT){
			if(!chnkChngOverflow){
				fprintf(stderr,"client chungusList Overflow!\n");
				chnkChngOverflow = false;
			}
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
	c->requested = 0;

	memset(c->chunks,0,16*16*16*sizeof(chunk *));
	return c;
}

void chungusFree(chungus *c){
	if(c == NULL){return;}
	animalDelChungus(c);
	itemDropDelChungus(c);
	fireDelChungus(c);
	throwableDelChungus(c);
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
	const vec coff = vecNew(c->x<<8,c->y<<8,c->z<<8);
	for(int x=0;x<16;x++){
		const int cx = x*CHUNK_SIZE+CHUNK_SIZE/2+(c->x<<8);
		for(int y=0;y<16;y++){
			const int cy = y*CHUNK_SIZE+CHUNK_SIZE/2+(c->y<<8);
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] == NULL){continue;}
				if(!chunkInFrustum(vecNew(x,y,z),coff)){continue;}
				const int cz = z*CHUNK_SIZE+CHUNK_SIZE/2+(c->z<<8);
				const float d = chunkDistance(cam->pos,vecNew(cx,cy,cz));
				if(d > renderDistance){continue;}
				drawQueue[*drawQueueLen].distance = d;
				drawQueue[*drawQueueLen].chnk = c->chunks[x][y][z];

				*drawQueueLen = *drawQueueLen+1;
			}
		}
	}
}

chunk *chungusGetChunk(chungus *c, u16 x,u16 y,u16 z){
	if(!inWorld(x,y,z)){return NULL;}
	return c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
}

chunk *chungusGetChunkOrNew(chungus *c, u16 x, u16 y, u16 z){
	chunk *chnk;
	const u16 cx = (x >> 4) & 0xF;
	const u16 cy = (y >> 4) & 0xF;
	const u16 cz = (z >> 4) & 0xF;
	chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew((c->x<<8)+(cx << 4),(c->y<<8)+(cy << 4),(c->z<<8)+(cz << 4));
	}
	return chnk;
}

u8 chungusGetB(chungus *c, u16 x,u16 y,u16 z){
	chunk *chnk = c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk == NULL){return 0;}
	return chnk->data[x&0xF][y&0xF][z&0xF];
}

void chungusSetB(chungus *c, u16 x,u16 y,u16 z,u8 block){
	chunk *chnk;
	u16 cx = (x >> 4) & 0xF;
	u16 cy = (y >> 4) & 0xF;
	u16 cz = (z >> 4) & 0xF;
	chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew((c->x<<8)+(cx << 4),(c->y<<8)+(cy << 4),(c->z<<8)+(cz << 4));
	}
	chunkSetB(chnk,x,y,z,block);
}

void chungusBoxF(chungus *c, u16 x,u16 y,u16 z, u16 w,u16 h,u16 d,u8 block){
	const u16 gx = (x+w)>>4;
	const u16 gy = (y+h)>>4;
	const u16 gz = (z+d)>>4;
	if((x|y|z)&(~0xFF)){return;}
	if(((x+w)|(y+h)|(z+d))&(~0xFF)){return;}
	u16 sx = x&0xF;
	u16 sw = CHUNK_SIZE;
	for(u16 cx=x>>4;cx<=gx;cx++){
		u16 sy = y&0xF;
		u16 sh = CHUNK_SIZE;
		if(cx == gx){
			sw = (x+w)&0xF;
		}
		for(u16 cy=y>>4;cy<=gy;cy++){
			u16 sz = z&0xF;
			u16 sd = CHUNK_SIZE;
			if(cy == gy){
				sh = (y+h)&0xF;
			}
			for(u16 cz=z>>4;cz<=gz;cz++){
				chunk *chnk = c->chunks[cx&0xF][cy&0xF][cz&0xF];
				if(chnk == NULL){
					c->chunks[cx&0xF][cy&0xF][cz&0xF] = chnk = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
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

void chungusBox(chungus *c, u16 x,u16 y,u16 z, u16 w,u16 h,u16 d,u8 block){
	for(u16 cx=0;cx<w;cx++){
	for(u16 cy=0;cy<h;cy++){
	for(u16 cz=0;cz<d;cz++){
		chungusSetB(c,cx+x,cy+y,cz+z,block);
	}
	}
	}
}

ivec chungusGetPos(const chungus *c){
	if(c == NULL){return ivecNOne();}
	return ivecNew(c->x,c->y,c->z);
}
