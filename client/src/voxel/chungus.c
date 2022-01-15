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

#define CHUNGUS_COUNT (1<<7)

chungus *chungusList;
uint     chungusCount = 0;
chungus *chungusFirstFree = NULL;

void chungusInit(){
	if(chungusList == NULL){
		chungusList = malloc(sizeof(chungus) * CHUNGUS_COUNT);
	}
	chungusCount     = 0;
	chungusFirstFree = NULL;
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

	memset(c,0,sizeof(chungus));
	c->x = x;
	c->y = y;
	c->z = z;

	const int cx = x << 8;
	const int cy = y << 8;
	const int cz = z << 8;
	for(int sx=0;sx<16;sx++){
	for(int sy=0;sy<16;sy++){
	for(int sz=0;sz<16;sz++){
		chunkReset(&c->chunks[sx][sy][sz], cx + (sx << 4), cy + (sy << 4), cz + (sz << 4));
	}
	}
	}

	return c;
}

void chungusFree(chungus *c){
	if(c == NULL){return;}
	animalDelChungus(c);
	itemDropDelChungus(c);
	throwableDelChungus(c);
	for(int x=0;x<16;x++){
	for(int y=0;y<16;y++){
	for(int z=0;z<16;z++){
		chunkFree(&c->chunks[x][y][z]);
	}
	}
	}
	c->nextFree = chungusFirstFree;
	chungusFirstFree = c;
}

void chungusQueueDraws(chungus *c,const character *cam, queueEntry *drawQueue,int *drawQueueLen){
	const vec coff = vecNew(c->x<<8,c->y<<8,c->z<<8);
	for(int x=0;x<16;x++){
		const int bx = x*CHUNK_SIZE+(c->x<<8);
		const int cx = bx+CHUNK_SIZE/2;
		const sideMask xMask =
			cam->pos.x < bx ? sideMaskLeft :
			cam->pos.x < bx + CHUNK_SIZE ? sideMaskLeft | sideMaskRight :
			sideMaskRight;
		for(int y=0;y<16;y++){
			const int by = y*CHUNK_SIZE+(c->y<<8);
			const int cy = by+CHUNK_SIZE/2;
			const sideMask yMask =
				cam->pos.y < by ? sideMaskBottom :
				cam->pos.y < by + CHUNK_SIZE ? sideMaskBottom | sideMaskTop :
				sideMaskTop;
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z].block == NULL){continue;}
				if(!chunkInFrustum(vecNew(x,y,z),coff)){continue;}
				const int bz = z*CHUNK_SIZE+(c->z<<8);
				const int cz = bz+CHUNK_SIZE/2;
				const sideMask zMask =
					cam->pos.z < bz ? sideMaskBack :
					cam->pos.z < bz + CHUNK_SIZE ? sideMaskBack | sideMaskFront :
					sideMaskFront;
				const float d = chunkDistance(cam->pos,vecNew(cx,cy,cz));
				if(d > renderDistance){continue;}
				drawQueue[*drawQueueLen].distance = d;
				drawQueue[*drawQueueLen].mask = xMask | yMask | zMask;
				drawQueue[*drawQueueLen].chnk = &c->chunks[x][y][z];

				*drawQueueLen = *drawQueueLen+1;
			}
		}
	}
}

chunk *chungusGetChunk(chungus *c, u16 x,u16 y,u16 z){
	if(!inWorld(x,y,z)){return NULL;}
	return &c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
}

chunk *chungusGetChunkOrNew(chungus *c, u16 x, u16 y, u16 z){
	const u16 cx = (x >> 4) & 0xF;
	const u16 cy = (y >> 4) & 0xF;
	const u16 cz = (z >> 4) & 0xF;
	return &c->chunks[cx][cy][cz];
}

blockId chungusGetB(chungus *c, u16 x,u16 y,u16 z){
	chunk *chnk = &c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->block == NULL){return 0;}
	return chnk->block->data[x&0xF][y&0xF][z&0xF];
}

void chungusSetB(chungus *c, u16 x,u16 y,u16 z,blockId block){
	u16 cx = (x >> 4) & 0xF;
	u16 cy = (y >> 4) & 0xF;
	u16 cz = (z >> 4) & 0xF;
	chunkSetB(&c->chunks[cx][cy][cz],x,y,z,block);
}

void chungusBoxF(chungus *c, u16 x,u16 y,u16 z, u16 w,u16 h,u16 d,blockId block){
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
				if(cz == gz){
					sd = (z+d)&0xF;
				}
				chunk *chnk = &c->chunks[cx&0xF][cy&0xF][cz&0xF];
				chunkBox(chnk,sx,sy,sz,sw,sh,sd,block);
				sz = 0;
			}
			sy = 0;
		}
		sx = 0;
	}
}

void chungusBox(chungus *c, u16 x,u16 y,u16 z, u16 w,u16 h,u16 d,blockId block){
	for(u16 cx=0;cx<w;cx++){
	for(u16 cy=0;cy<h;cy++){
	for(u16 cz=0;cz<d;cz++){
		chungusSetB(c,cx+x,cy+y,cz+z,block);
	}
	}
	}
}

vec chungusGetPos(const chungus *c){
	if(c == NULL){return vecNOne();}
	return vecNew(c->x,c->y,c->z);
}
