#include "chungus.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../persistence/savegame.h"
#include "../worldgen/worldgen.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>


chungus chungusList[1 << 14];
uint chungusCount = 0;
chungus *chungusFirstFree = NULL;
u64 freeTime = 0;

float chunkDistance(const entity *cam, const vec pos){
	return vecMag(vecSub(pos,cam->pos));
}

void chungusSetClientUpdated(chungus *c,u64 updated){
	c->clientsUpdated = updated;
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] == NULL){continue;}
				c->chunks[x][y][z]->clientsUpdated = updated;
			}
		}
	}
}

chungus *chungusNew(u8 x, u8 y, u8 z){
	chungus *c = NULL;
	if(y > 128){
		fprintf(stderr,"Y seems a bit high!\n");
	}
	if((x < 64) || (z < 64)){
		fprintf(stderr,"warn\n");
	}

	if(chungusFirstFree == NULL){
		if(chungusCount >= (int)(sizeof(chungusList) / sizeof(chungus))-1){
			fprintf(stderr,"chungus load shedding [%u chungi]!\n",chungusCount);
			chungusFreeOldChungi(1000);
			if(chungusFirstFree == NULL){
				fprintf(stderr,"server chungusList Overflow!\n");
				return NULL;
			}else{
				fprintf(stderr,"chungusList overflow averted, freed some memory!\n");
				c = chungusFirstFree;
				chungusFirstFree = c->nextFree;
			}
		}else{
			c = &chungusList[chungusCount++];
		}
	}else{
		c = chungusFirstFree;
		chungusFirstFree = c->nextFree;
	}

	c->x = x;
	c->y = y;
	c->z = z;
	c->freeTimer = freeTime;
	c->nextFree = NULL;
	c->spawn = ivecNOne();
	c->clientsSubscribed  = (u64)1 << 63;
	c->clientsUpdated     = (u64)1 << 31;

	memset(c->chunks,0,16*16*16*sizeof(chunk *));

	return c;
}

void chungusWorldGenLoad(chungus *c){
	worldgen *wgen = worldgenNew(c);
	worldgenGenerate(wgen);
	worldgenFree(wgen);
	chungusSetClientUpdated(c,(u64)1 << 31);
	chungusLoad(c);
}

void chungusFree(chungus *c){
	if(c == NULL){return;}
	fprintf(stderr,"ChungusFree[] %p %i:%i:%i\n",c,c->x,c->y,c->z);
	chungusSave(c);
	animalDelChungus(c);
	itemDropDelChungus(c);
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				chunkFree(c->chunks[x][y][z]);
			}
		}
	}
	memset(c->chunks,0,16*16*16*sizeof(chunk *));
	c->nextFree = chungusFirstFree;
	chungusFirstFree = c;
}

chunk *chungusGetChunk(chungus *c, int x,int y,int z){
	return c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
}

u8 chungusGetB(chungus *c, int x,int y,int z){
	c->freeTimer = freeTime;
	chunk *chnk = c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk == NULL)        { return 0; }
	return chnk->data[x&0xF][y&0xF][z&0xF];
}

int chungusGetHighestP(chungus *c, int x, int *retY, int z){
	int cx = (x >> 4) & 0xF;
	int cz = (z >> 4) & 0xF;
	x &= 0xF;
	z &= 0xF;

	for(int cy=15;cy >= 0;cy--){
		chunk *chnk = c->chunks[cx][cy][cz];
		if(chnk == NULL){continue;}
		for(int y=CHUNK_SIZE-1;y>=0;y--){
			u8 b = chnk->data[x&0xF][y&0xF][z&0xF];
			if(b != 0){
				*retY = (cy*CHUNK_SIZE)+y;
				return 1;
			}
		}
	}
	return 0;
}

void chungusSetB(chungus *c, int x,int y,int z,u8 block){
	c->freeTimer = freeTime;
	int cx = (x >> 4) & 0xF;
	int cy = (y >> 4) & 0xF;
	int cz = (z >> 4) & 0xF;
	chunk *chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		chnk = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
		c->chunks[cx][cy][cz] = chnk;
	}
	chunkSetB(chnk,x,y,z,block);
	c->clientsUpdated = 0;
}

void chungusBoxF(chungus *c,int x,int y,int z,int w,int h,int d,u8 block){
	c->freeTimer = freeTime;
	int gx = (x+w)>>4;
	int gy = (y+h)>>4;
	int gz = (z+d)>>4;

	if( (x  |  y  |  z  ) &(~0xFF)) { return; }
	if(((x+w)|(y+h)|(z+d))&(~0xFF)) { return; }

	int sx = x & 0xF;
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
					chnk = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
					c->chunks[cx&0xF][cy&0xF][cz&0xF] = chnk;
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

void chungusBox(chungus *c, int x,int y,int z, int w,int h,int d,u8 block){
	c->freeTimer = freeTime;
	if(w < 0){ chungusBox(c,x+w,y,z,-w,h,d,block); }
	if(h < 0){ chungusBox(c,x,y+h,z,w,-h,d,block); }
	if(d < 0){ chungusBox(c,x,y,z+d,w,h,-d,block); }
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				chungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
	c->clientsUpdated = 0;
}

void chungusBoxIfEmpty(chungus *c, int x,int y,int z, int w,int h,int d,u8 block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				if(chungusGetB(c,cx+x,cy+y,cz+z)){continue;}
				chungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
	c->clientsUpdated = 0;
}


void chungusRoughBox(chungus *c,int x,int y,int z,int w,int h,int d,u8 block){
	int dx = x+w-1;
	int dy = y+h-1;
	int dz = z+d-1;

	for(int cx=x;cx<=dx;cx++){
		for(int cy=y;cy<=dy;cy++){
			for(int cz=z;cz<=dz;cz++){
				if((cx == x) || (cx == dx) || (cy == y) || (cy == dy) || (cz == z) || (cz == dz)){
					if((rngValR()&0x100) == 0){continue;}
				}
				chungusSetB(c,cx,cy,cz,block);
			}
		}
	}
	c->clientsUpdated = 0;
}

void chungusBoxSphere(chungus *c, int x, int y, int z, int r, u8 block){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
		for(int cy=-r;cy<=r;cy++){
			for(int cz=-r;cz<=r;cz++){
				const int d = (cx*cx)+(cy*cy)+(cz*cz);
				if(d >= md){continue;}
				chungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void chungusFill(chungus *c, int x,int y,int z,u8 b){
	int cx = (x / CHUNK_SIZE) & 0xF;
	int cy = (y / CHUNK_SIZE) & 0xF;
	int cz = (z / CHUNK_SIZE) & 0xF;
	chunk *chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
	}
	chunkFill(chnk,b);
	c->clientsUpdated = 0;
}

void chungusSubscribePlayer(chungus *c, uint p){
	if(c == NULL){return;}
	u64 mask = ~(1 << p);
	if(c->clientsSubscribed & mask){return;}

	c->clientsSubscribed |= 1 << p;
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] != NULL){
					c->chunks[x][y][z]->clientsUpdated &= mask;
				}
			}
		}
	}
}

void chungusSetAllUpdated(chungus *c, u64 nUpdated){
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] != NULL){
					c->chunks[x][y][z]->clientsUpdated = nUpdated;
				}
			}
		}
	}
}

int chungusUnsubscribePlayer(chungus *c, uint p){
	if(c == NULL){return 0;}
	u32 mask = ~(1 << p);

	c->clientsSubscribed &= mask;
	c->clientsUpdated    &= mask;
	for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
			for(int z=0;z<16;z++){
				if(c->chunks[x][y][z] != NULL){
					c->chunks[x][y][z]->clientsUpdated &= mask;
				}
			}
		}
	}

	return 0;
}

uint chungusIsSubscribed(chungus *c, uint p){
	if(c == NULL){return 0;}

	return c->clientsSubscribed & (1 << p);
}

uint chungusIsUpdated(chungus *c, uint p){
	if(c == NULL){return 1;}

	return c->clientsUpdated & (1 << p);
}

void chungusSetUpdated(chungus *c, uint p){
	if( c == NULL){return;}

	c->clientsUpdated |= 1 << p;
}

void chungusUnsetUpdated(chungus *c, uint p){
	if(c == NULL){return;}
	c->clientsUpdated &= ~(1 << p);
}

int chungusUpdateClient(chungus *c, uint p){
	if(c == NULL)                          { return 0; }
	if(!(c->clientsSubscribed & (1 << p))) { return 1; }
	if( chungusIsUpdated(c,p))             { return 0; }

	addChungusToQueue(p,c->x,c->y,c->z);
	return 0;
}

uint chungusFreeOldChungi(u64 threshold){
	const u64 curTicks = getTicks();
	uint ret = 0;

	for(uint i=0;i<chungusCount;i++){
		chungus *chng = &chungusList[i];
		if(chng->nextFree != NULL)      {continue;}
		if(chng->clientsSubscribed != 0){continue;}
		if(curTicks < chng->freeTimer + threshold){continue;}
		const u8 x = chng->x;
		const u8 y = chng->y;
		const u8 z = chng->z;
		if(y >= 128){
			fprintf(stderr,"Y >= 128, something went wrong!!! [%u,%u,%u]\n",x,y,z);
			continue;
		}
		if((x >= 127) && (x <= 129) && (y <= 3) && (z >= 127) && (z <= 129)){continue;}
		bigchungusFreeChungus(&world,chng->x,chng->y,chng->z);
		ret++;
	}
	return ret;
}

float chungusDistance(const character *cam, const chungus *chng){
	if(cam == NULL) {return 8192.f;}
	if(chng == NULL){return 8192.f;}
	const uint x = (chng->x << 8) + 128;
	const uint y = (chng->y << 8) + 128;
	const uint z = (chng->z << 8) + 128;
	return vecMag(vecSub(vecNew(x,y,z),cam->pos));
}

void chungusUnsubFarChungi(){
	static u64 lastCall = 0;
	const u64 curTicks = getTicks();
	if(curTicks < lastCall + 1000){return;}
	lastCall = curTicks;

	for(uint i=0;i<chungusCount;i++){
		chungus *chng = &chungusList[i];
		if(chng->nextFree != NULL){continue;}
		const u8 x = chng->x;
		const u8 y = chng->y;
		const u8 z = chng->z;
		if(y >= 128){
			fprintf(stderr,"Y >= 128, something went wrong!!! [%u,%u,%u]\n",x,y,z);
			continue;
		}
		chng->clientsSubscribed &= 0xFFFFFFFF;

		for(uint ii=0;ii<clientCount;++ii){
			const float cdist = chungusDistance(clients[ii].c,chng);
			if(cdist < 384.f){
				chungusSubscribePlayer(chng,ii);
			}else if(cdist > 768.f){
				chungusUnsubscribePlayer(chng,ii);
			}
		}
	}
}
