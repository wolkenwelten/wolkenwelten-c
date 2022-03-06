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

#include "chunk.h"

#include "../game/being.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/chunkOverlay.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__HAIKU__) || defined(__EMSCRIPTEN__) || defined(__OpenBSD__)
#define CHUNK_COUNT (1<<17)
#else
#define CHUNK_COUNT (1<<18)
#endif

float chunkDistance(const vec pos, const chunk *chnk){
	const vec chunkpos = vecNew(chnk->x,chnk->y,chnk->z);
	return vecMag(vecSub(chunkpos,pos));
}

void chunkReset(chunk *c, u16 x, u16 y, u16 z){
	c->x = x & (~0xF);
	c->y = y & (~0xF);
	c->z = z & (~0xF);
	c->clientsUpdated = 1 << 31;
	chungus *chng = worldTryChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){
		beingListInit(&c->bl,NULL);
	}else{
		beingListInit(&c->bl,&chng->bl);
	}
	c->block = NULL;
	c->fluid = NULL;
	c->flame = NULL;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	if(c->fluid){
		chunkOverlayFree(c->fluid);
		c->fluid = NULL;
	}
	if(c->block){
		chunkOverlayFree(c->block);
		c->block = NULL;
	}
}

void chunkSetB(chunk *c,int x,int y,int z,blockId block){
	if(c->block == NULL){c->block = chunkOverlayAllocate();}
	c->block->data[x&0xF][y&0xF][z&0xF] = block;
	c->clientsUpdated = 0;
}

void chunkFill(chunk *c, blockId b){
	if(!c){return;}
	if(c->block == NULL){c->block = chunkOverlayAllocate();}
	memset(c->block->data,b,CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
	c->clientsUpdated = 0;
}

int chunkIsUpdated(chunk *c, uint p){
	if(c == NULL){return 1;}
	return c->clientsUpdated & (1 << p);
}
void chunkSetUpdated(chunk *c, uint p){
	if(c == NULL){return;}
	c->clientsUpdated |= 1 << p;
}
void chunkUnsetUpdated(chunk *c, uint p){
	if(c == NULL){return;}
	c->clientsUpdated &= ~(1 << p);
}

void chunkBox(chunk *c, int x,int y,int z,int gx,int gy,int gz,blockId block){
	if(c->block == NULL){c->block = chunkOverlayAllocate();}
	for(int cx = x; cx < gx; cx++){
	for(int cy = y; cy < gy; cy++){
	for(int cz = z; cz < gz; cz++){
		c->block->data[cx][cy][cz] = block;
	}
	}
	}
	c->clientsUpdated = 0;
}
