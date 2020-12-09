#include "chunk.h"

#include "../voxel/chungus.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHUNK_COUNT (1<<18)

chunk *chunkList;
uint   chunkFreeCount = 0;
uint   chunkCount     = 0;
chunk *chunkFirstFree = NULL;

void chunkInit(){
	chunkList = malloc(sizeof(chunk) * CHUNK_COUNT);
	//memset(chunkList,0,sizeof(chunkList));
}

float chunkDistance(const vec pos, const chunk *chnk){
	const vec chunkpos = vecNew(chnk->x,chnk->y,chnk->z);
	return vecMag(vecSub(chunkpos,pos));
}

chunk *chunkNew(u16 x,u16 y,u16 z){
	chunk *c = NULL;
	if(chunkFirstFree == NULL){
		//fprintf(stderr,"chunkCount:%u\n",chunkCount);
		if(chunkCount >= CHUNK_COUNT){
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
				if(chunkFreeCount > (1<<18)){
					printf("SOMETHING HAPPENED!!!");
				}
			}
		}else{
			c = &chunkList[chunkCount++];
		}
	}else{
		//fprintf(stderr,"Recycling %u\n",chunkCount);
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
