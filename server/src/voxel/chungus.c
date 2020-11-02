#include "chungus.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../persistence/savegame.h"
#include "../worldgen/worldgen.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>


chungus chungusList[1 << 12];
uint chungusCount = 0;
chungus *chungusFirstFree = NULL;

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

	if(chungusFirstFree == NULL){
		if(chungusCount >= (int)(sizeof(chungusList) / sizeof(chungus))-1){
			fprintf(stderr,"server chungusList Overflow!\n");
			return NULL;
		}
		c = &chungusList[chungusCount++];
	}else{
		c = chungusFirstFree;
		chungusFirstFree = c->nextFree;
	}

	c->x = x;
	c->y = y;
	c->z = z;
	c->nextFree = NULL;
	c->spawn = ivecNOne();
	c->clientsSubscribed  = 0;
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
				c->chunks[x][y][z] = NULL;
			}
		}
	}
	c->nextFree = chungusFirstFree;
	chungusFirstFree = c;
}

chunk *chungusGetChunk(chungus *c, int x,int y,int z){
	if(((x|y|z)>>4)&(~0xF)){return NULL;}
	return c->chunks[x>>4][y>>4][z>>4];
}

u8 chungusGetB(chungus *c, int x,int y,int z){
	if(((x|y|z)>>4)&(~0xF)) { return 0; }
	chunk *chnk = c->chunks[x>>4][y>>4][z>>4];
	if(chnk == NULL)        { return 0; }

	return chnk->data[x&0xF][y&0xF][z&0xF];
}

int chungusGetHighestP(chungus *c, int x, int *retY, int z){
	if((x|z)&(~0xFF)){return 0;}
	int cx = x >> 4;
	int cz = z >> 4;
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
	if((x|y|z)&(~0xFF)){return;}
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

	c->clientsSubscribed |= 1 << p;
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
	chungusSetUpdated(c,p);
	return 0;

}
