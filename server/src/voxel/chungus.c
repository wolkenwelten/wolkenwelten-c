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
#include "../game/animal.h"
#include "../game/being.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/throwable.h"
#include "../network/server.h"
#include "../persistence/savegame.h"
#include "../worldgen/worldgen.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>

#define CHUNGUS_COUNT (1<<14)

chungus *chungusList;
uint chungusCount = 0;
uint chungusFreeCount = 0;
chungus *chungusFirstFree = NULL;
u64 freeTime = 0;

void chungusInit(){
	chungusList = malloc(sizeof(chungus) * CHUNGUS_COUNT);
}

void chungusSetClientUpdated(chungus *c,u64 updated){
	c->clientsUpdated = updated;
	for(int x=0;x<CHUNGUS_COORDS;x++){
	for(int y=0;y<CHUNGUS_COORDS;y++){
	for(int z=0;z<CHUNGUS_COORDS;z++){
		if(c->chunks[x][y][z] == NULL){continue;}
		c->chunks[x][y][z]->clientsUpdated = updated;
	}
	}
	}
}

chungus *chungusNew(u8 x, u8 y, u8 z){
	chungus *c = NULL;
	if(y > 128){
		fprintf(stderr,"Y seems a bit high! %u %u %u\n",x,y,z);
	}
	if((x < 64) || (z < 64)){
		fprintf(stderr,"X/Z seems low, warn %u %u %u\n",x,y,z);
	}

	if(chungusFirstFree == NULL){
		if(chungusCount >= CHUNGUS_COUNT){
			fprintf(stderr,"chungus load shedding [%u chungi]!\n",chungusCount);
			chungusFreeOldChungi(1000);
			if(chungusFirstFree == NULL){
				fprintf(stderr,"server chungusList Overflow!\n");
				return NULL;
			}else{
				fprintf(stderr,"chungusList overflow averted, freed some memory!\n");
				c = chungusFirstFree;
				chungusFirstFree = c->nextFree;
				chungusFreeCount--;
			}
		}else{
			c = &chungusList[chungusCount++];
		}
	}else{
		c = chungusFirstFree;
		chungusFirstFree = c->nextFree;
		chungusFreeCount--;
	}

	c->x = x;
	c->y = y;
	c->z = z;
	c->nextFree = NULL;
	c->freeTimer = freeTime;
	c->sx = c->sy = c->sz = 0;
	c->clientsSubscribed = (u64)1 << 63;
	c->clientsUpdated    = (u64)1 << 31;
	beingListInit(&c->bl,NULL);

	memset(c->chunks,0,16*16*16*sizeof(chunk *));
	chunkCheckShed();

	return c;
}

void chungusWorldGenLoad(chungus *c){
	chunkCheckShed();
	worldgen *wgen = worldgenNew(c);
	worldgenGenerate(wgen);
	worldgenFree(wgen);
	chungusSetClientUpdated(c,(u64)1 << 31);
	chungusLoad(c);
}

void chungusFree(chungus *c){
	if(c == NULL){return;}
	chungusSave(c);
	animalDelChungus(c);
	itemDropDelChungus(c);
	fireDelChungus(c);
	throwableDelChungus(c);
	for(int x=0;x<CHUNGUS_COORDS;x++){
	for(int y=0;y<CHUNGUS_COORDS;y++){
	for(int z=0;z<CHUNGUS_COORDS;z++){
		chunkFree(c->chunks[x][y][z]);
	}
	}
	}
	memset(c->chunks,0,sizeof(c->chunks));
	c->nextFree = chungusFirstFree;
	chungusFirstFree = c;
	chungusFreeCount++;
}

chunk *chungusGetChunk(chungus *c, int x,int y,int z){
	return c->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
}

blockId chungusGetB(chungus *c, int x,int y,int z){
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
			blockId b = chnk->data[x&0xF][y&0xF][z&0xF];
			if(b != 0){
				*retY = (cy*CHUNK_SIZE)+y;
				return 1;
			}
		}
	}
	return 0;
}

void chungusSetB(chungus *c, int x,int y,int z,blockId block){
	if((x&(~0xFF)) || (y&(~0xFF)) || (z&(~0xFF))){return;}
	c->freeTimer = freeTime;
	int cx = x >> 4;
	int cy = y >> 4;
	int cz = z >> 4;
	chunk *chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
		if(chnk == NULL){
			chungusFreeOldChungi(1000);
			c->chunks[cx][cy][cz] = chnk = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
			if(chnk == NULL){return;}
		}
	}
	chunkSetB(chnk,x,y,z,block);
	c->clientsUpdated = 0;
}

void chungusBoxF(chungus *c,int x,int y,int z,int w,int h,int d,blockId block){
	c->freeTimer = freeTime;
	int gx = (x+w)>>4;
	int gy = (y+h)>>4;
	int gz = (z+d)>>4;

	if( (x   | y   | z  ) &(~0xFF)) { return; }
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

void chungusBoxFWG(chungus *c,int x,int y,int z,int w,int h,int d){
	const int gx = (x+w)>>4;
	const int gy = (y+h)>>4;
	const int gz = (z+d)>>4;
	blockId block = I_Stone;

	if( (x   | y   | z  ) &(~0xFF)){ return; }
	if(((x+w)|(y+h)|(z+d))&(~0xFF)){ return; }
	const int bx = x>>4;
	const int by = y>>4;
	const int bz = z>>4;

	int sx = x & 0xF;
	int sw = CHUNK_SIZE;
	for(int cx=bx;cx<=gx;cx++){
		if(cx == gx){sw = (x+w)&0xF;}
		int sy = y&0xF;
		int sh = CHUNK_SIZE;
		for(int cy=by;cy<=gy;cy++){
			if(cy == gy){sh = (y+h)&0xF;}
			int sz = z&0xF;
			int sd = CHUNK_SIZE;
			for(int cz=bz;cz<=gz;cz++){
				if(cz == gz){sd = (z+d)&0xF;}
				chunk *chnk = c->chunks[cx&0xF][cy&0xF][cz&0xF];
				if(chnk == NULL){
					chnk = c->chunks[cx&0xF][cy&0xF][cz&0xF] = chunkNew((c->x<<8)+(cx<<4),(c->y<<8)+(cy<<4),(c->z<<8)+(cz<<4));
				}
				chunkBox(chnk,sx,sy,sz,sw,sh,sd,block);
				sz = 0;
			}
			sy = 0;
		}
		sx = 0;
	}
}

void chungusBox(chungus *c, int x,int y,int z, int w,int h,int d,blockId block){
	c->freeTimer = freeTime;
	if(w < 0){ chungusBox(c,x+w,y  ,z  ,-w, h, d,block);}
	if(h < 0){ chungusBox(c,x  ,y+h,z  , w,-h, d,block);}
	if(d < 0){ chungusBox(c,x  ,y  ,z+d, w, h,-d,block);}
	for(int cx=0; cx < w;cx++){
	for(int cy=0; cy < h;cy++){
	for(int cz=0; cz < d;cz++){
		chungusSetB(c,cx+x,cy+y,cz+z,block);
	}
	}
	}
	c->clientsUpdated = 0;
}

void chungusBoxIfEmpty(chungus *c, int x,int y,int z, int w,int h,int d,blockId block){
	const int cw = x+w;
	const int ch = y+h;
	const int cd = z+d;
	for(int cx=x;cx < cw;cx++){
	for(int cy=y;cy < ch;cy++){
	for(int cz=z;cz < cd;cz++){
		if(chungusGetB(c,cx,cy,cz)){continue;}
		chungusSetB(c,cx,cy,cz,block);
		c->clientsUpdated = 0;
	}
	}
	}
}

void chungusBoxSphere(chungus *c, int x, int y, int z, int r, blockId block){
	const int md = r*r;
	const int sx = MAX(0,-r + x);
	const int ex = MIN(CHUNGUS_SIZE-1,r + x);
	const int sy = MAX(0,-r + y);
	const int ey = MIN(CHUNGUS_SIZE-1,r + y);
	const int sz = MAX(0,-r + z);
	const int ez = MIN(CHUNGUS_SIZE-1,r + z);

	for(int cx = sx; cx <= ex; cx++){
	for(int cy = sy; cy <= ey; cy++){
	for(int cz = sz; cz <= ez; cz++){
		const int ox = cx - x;
		const int oy = cy - y;
		const int oz = cz - z;
		const int d = (ox*ox)+(oy*oy)+(oz*oz);
		if(d >= md){continue;}
		chungusSetB(c,cx,cy,cz,block);
	}
	}
	}
}

void chungusFill(chungus *c, int x,int y,int z,blockId b){
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
	if(c->clientsSubscribed & (1 << p)){return;}

	c->clientsSubscribed |= 1 << p;
	for(int x = 0; x < CHUNGUS_COORDS; x++){
	for(int y = 0; y < CHUNGUS_COORDS; y++){
	for(int z = 0; z < CHUNGUS_COORDS; z++){
		if(c->chunks[x][y][z] == NULL){ continue; }
		c->chunks[x][y][z]->clientsUpdated &= mask;
		beingListSync(p, &c->chunks[x][y][z]->bl);
	}
	}
	}
	beingListSync(p, &c->bl);
}

void chungusSetAllUpdated(chungus *c, u64 nUpdated){
	for(int x = 0; x < CHUNGUS_COORDS; x++){
	for(int y = 0; y < CHUNGUS_COORDS; y++){
	for(int z = 0; z < CHUNGUS_COORDS; z++){
		if(c->chunks[x][y][z] == NULL){ continue; }
		c->chunks[x][y][z]->clientsUpdated = nUpdated;
	}
	}
	}
}

int chungusUnsubscribePlayer(chungus *c, uint p){
	if(c == NULL){return 0;}
	u32 mask = ~(1 << p);

	c->clientsSubscribed &= mask;
	c->clientsUpdated    &= mask;
	for(int x = 0; x < CHUNGUS_COORDS; x++){
	for(int y = 0; y < CHUNGUS_COORDS; y++){
	for(int z = 0; z < CHUNGUS_COORDS; z++){
		if(c->chunks[x][y][z] == NULL){ continue; }
		c->chunks[x][y][z]->clientsUpdated &= mask;
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
	bool chungusUpdated = true;
	bool fullUpdate = (clients[p].chnkUpdateIter & 0xFFF) == 0;
	if(clients[p].chnkUpdateIter & 0x3){ return 0;}

	for(uint x=0;x<CHUNGUS_COORDS;x++){
	for(uint y=0;y<CHUNGUS_COORDS;y++){
	for(uint z=0;z<CHUNGUS_COORDS;z++){
		chunk *chnk = c->chunks[x][y][z];
		if(chnk == NULL){continue;}
		if(!fullUpdate){
			const float d = chunkDistance(clients[p].c->pos,chnk);
			if(d > 32.f){
				chungusUpdated = false;
				continue;
			}
		}
		msgSendChunk(p,chnk);
		chunkSetUpdated(chnk,p);
	}
	}
	}
	if(chungusUpdated){chungusSetUpdated(c,p);}
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
		const blockId x = chng->x;
		const blockId y = chng->y;
		const blockId z = chng->z;
		if(y >= 128){
			fprintf(stderr,"Y >= 128, something went wrong!!! [%u,%u,%u]\n",x,y,z);
			continue;
		}
		if((x >= 127) && (x <= 129) && (y <= 3) && (z >= 127) && (z <= 129)){continue;}
		bigchungusFreeChungus(&world,chng->x,chng->y,chng->z);
		if(++ret > 8){break;}
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
		const blockId x = chng->x;
		const blockId y = chng->y;
		const blockId z = chng->z;
		if(y >= 128){
			fprintf(stderr,"Y >= 128, something went wrong!!! [%u,%u,%u]\n",x,y,z);
			continue;
		}
		chng->clientsSubscribed &= 0xFFFFFFFF;

		for(uint ii=0;ii<clientCount;++ii){
			if(clients[ii].state){continue;}
			const float cdist = chungusDistance(clients[ii].c,chng);

			if(cdist < 256.f){
				chungusSubscribePlayer(chng,ii);
			}else if(cdist > 2048.f){
				chungusUnsubscribePlayer(chng,ii);
			}
		}
	}
}

vec chungusGetPos(const chungus *c){
	if(c == NULL){return vecNOne();}
	return vecNew(c->x,c->y,c->z);
}
