#include "chunk.h"

#include "../../../common/src/game/blockType.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

chunk  chunkList[1 << 18];
uint   chunkFreeCount = 0;
uint   chunkCount     = 0;
chunk *chunkFirstFree       = NULL;

chunk *chunkNew(u16 x,u16 y,u16 z){
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

u8 *chunkSave(chunk *c, u8 *buf){
	if((c->clientsUpdated & ((uint64_t)1 << 31)) != 0){return buf;}
	buf[0] = 0x01;
	buf[1] = (c->x >> 4)&0xF;
	buf[2] = (c->y >> 4)&0xF;
	buf[3] = (c->z >> 4)&0xF;
	memcpy(buf+4,c->data,16*16*16);
	return buf+4100;
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
