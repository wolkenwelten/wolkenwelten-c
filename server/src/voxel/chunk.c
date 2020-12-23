#include "chunk.h"

#include "../game/being.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHUNK_COUNT (1<<20)

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
	c->clientsUpdated = (u64)1 << 31;
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
