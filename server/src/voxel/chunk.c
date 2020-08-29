#include "chunk.h"

#include "../../../common/src/game/blockType.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

chunk chunkList[1 << 18];
unsigned int chunkFreeCount = 0;
unsigned int chunkCount     = 0;
chunk *chunkFirstFree       = NULL;

chunk *chunkNew(uint16_t x,uint16_t y,uint16_t z){
	chunk *c = NULL;
	if(chunkFirstFree == NULL){
		if(chunkCount >= (int)(sizeof(chunkList) / sizeof(chunk))-1){
			fprintf(stderr,"server chunkList Overflow!\n");
			return NULL;
		}
		c = &chunkList[chunkCount++];
	}else{
		c = chunkFirstFree;
		chunkFirstFree = c->nextFree;
		chunkFreeCount--;
	}
	c->x = x & (~0xF);
	c->y = y & (~0xF);
	c->z = z & (~0xF);
	c->nextFree = NULL;
	c->clientsUpdated = 1 << 63;
	
	memset(c->data,0,sizeof(c->data));
	return c;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	chunkFreeCount++;
	c->nextFree = chunkFirstFree;
	chunkFirstFree = c;
}

void chunkBox(chunk *c, int x,int y,int z,int gx,int gy,int gz,uint8_t block){
	for(int cx=x;cx<gx;cx++){
		for(int cy=y;cy<gy;cy++){
			for(int cz=z;cz<gz;cz++){
				c->data[cx][cy][cz] = block;
			}
		}
	}
}

void chunkSetB(chunk *c,int x,int y,int z,uint8_t block){
	c->data[x&0xF][y&0xF][z&0xF] = block;
	c->clientsUpdated = 0;
}

void chunkFill(chunk *c, uint8_t b){
	memset(c->data,b,CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
	c->clientsUpdated = 0;
}

unsigned int chunkGetFree(){
	return chunkFreeCount;
}
unsigned int chunkGetActive(){
	return chunkCount;
}
int chunkIsUpdated(chunk *c, int p){
	if(c == NULL){return 1;}
	return c->clientsUpdated & (1 << p);
}
void chunkSetUpdated(chunk *c, int p){
	if(c == NULL){return;}
	c->clientsUpdated |= 1 << p;
}
