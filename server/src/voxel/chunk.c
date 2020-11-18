#include "chunk.h"

#include "../voxel/chungus.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

chunk *chunkList;
uint   chunkFreeCount = 0;
uint   chunkCount     = 0;
chunk *chunkFirstFree = NULL;

#ifdef __EMSCRIPTEN__
const uint chunkListSize = (1<<20);
#else
const uint chunkListSize = (1<<20);
#endif

void chunkInit(){
	chunkList = malloc(sizeof(chunk) * chunkListSize);
	if(chunkList == NULL){
		fprintf(stderr,"Error allocating chunkList, exit\n");
		exit(1);
	}
}

chunk *chunkNew(u16 x,u16 y,u16 z){
	chunk *c = NULL;
	if(chunkFirstFree == NULL){
		if(chunkCount >= chunkListSize){
			fprintf(stderr,"chunk load shedding [%u / %u chunks]!\n",chunkFreeCount,chunkCount);
			uint chngFree = chungusFreeOldChungi(1000);
			if(chunkFirstFree == NULL){
				fprintf(stderr,"server chunkList Overflow!\n");
				return NULL;
			}else{
				fprintf(stderr,"chunkList overflow averted, freed some memory [%u chungi | %u chunks]!\n",chngFree,chunkFreeCount);
				c = chunkFirstFree;
				chunkFirstFree = c->nextFree;
				chunkFreeCount--;
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
	c->nextFree = NULL;
	c->clientsUpdated = (u64)1 << 31;

	memset(c->data,0,sizeof(c->data));
	return c;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	chunkFreeCount++;
	c->nextFree = chunkFirstFree;
	chunkFirstFree = c;
}

void chunkBox(chunk *c, int x,int y,int z,int gx,int gy,int gz,u8 block){
	for(int cx=x;cx<gx;cx++){
		for(int cy=y;cy<gy;cy++){
			for(int cz=z;cz<gz;cz++){
				c->data[cx][cy][cz] = block;
			}
		}
	}
	c->clientsUpdated = 0;
}

void chunkSetB(chunk *c,int x,int y,int z,u8 block){
	c->data[x&0xF][y&0xF][z&0xF] = block;
	c->clientsUpdated = 0;
}

void chunkFill(chunk *c, u8 b){
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
