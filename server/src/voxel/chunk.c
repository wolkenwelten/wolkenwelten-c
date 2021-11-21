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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__HAIKU__) || defined(__EMSCRIPTEN__) || defined(__OpenBSD__)
#define CHUNK_COUNT (1<<17)
#else
#define CHUNK_COUNT (1<<18)
#endif

chunk *chunkList;
uint   chunkFreeCount = 0;
uint   chunkCount     = 0;
chunk *chunkFirstFree = NULL;

void chunkInit(){
	chunkList = malloc(sizeof(chunk) * CHUNK_COUNT);
}

float chunkDistance(const vec pos, const chunk *chnk){
	const vec chunkpos = vecNew(chnk->x,chnk->y,chnk->z);
	return vecMag(vecSub(chunkpos,pos));
}

void chunkCheckShed(){
	if((chunkFreeCount+(chunkCount - CHUNK_COUNT)) > 5000){return;}
	uint chngFree = chungusFreeOldChungi(1000);
	fprintf(stderr,"[SRV] preemptive chunk load shedding! [Free:%i Used:%i]\n",chunkFreeCount,chunkCount);
	fprintf(stderr,"[SRV] disaster  averted, freed some memory [%u chungi | %u chunks]!\n",chngFree,chunkFreeCount);
}

chunk *chunkNew(u16 x,u16 y,u16 z){
	chunk *c = NULL;
	if(chunkFirstFree == NULL){
		if(chunkCount >= CHUNK_COUNT){
			fprintf(stderr,"[SRV] chunk load shedding [%u / %u chunks]!\n",chunkFreeCount,chunkCount);
			uint chngFree = chungusFreeOldChungi(1000);
			if(chunkFirstFree == NULL){
				fprintf(stderr,"[SRV] server chunkList Overflow!\n");
				return NULL;
			}else{
				fprintf(stderr,"[SRV] chunkList overflow averted, freed some memory [%u chungi | %u chunks]!\n",chngFree,chunkFreeCount);
				c = chunkFirstFree;
				chunkFirstFree = c->nextFree;
				chunkFreeCount--;
				if(chunkFreeCount > (1<<18)){
					printf("SOMETHING HAPPENED!!!");
				}
			}
		}else{
			c = &chunkList[chunkCount++];
		}
	}else{
		c = chunkFirstFree;
		chunkFirstFree = c->nextFree;
		chunkFreeCount--;
	}
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

	memset(c->data,0,sizeof(c->data));
	return c;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	chunkFreeCount++;
	c->nextFree = chunkFirstFree;
	c->bl.parent = NULL;
	chunkFirstFree = c;
}

void chunkSetB(chunk *c,int x,int y,int z,blockId block){
	c->data[x&0xF][y&0xF][z&0xF] = block;
	c->clientsUpdated = 0;
}

void chunkFill(chunk *c, blockId b){
	memset(c->data,b,CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
	c->clientsUpdated = 0;
}

uint chunkGetFree(){
	return chunkFreeCount;
}
uint chunkGetActive(){
	return chunkCount;
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
	for(int cx = x; cx < gx; cx++){
	for(int cy = y; cy < gy; cy++){
	for(int cz = z; cz < gz; cz++){
		c->data[cx][cy][cz] = block;
	}
	}
	}
	c->clientsUpdated = 0;
}
