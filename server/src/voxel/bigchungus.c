#include "bigchungus.h"

#include "../network/server.h"
#include "../game/blockMining.h"
#include "../misc/options.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/noise.h"

#include <stdio.h>
#include <string.h>

bigchungus world;

void bigchungusInit(bigchungus *c){
	//memset(c->chungi,0,256*128*256*sizeof(chunk *));
	generateNoise(optionWorldSeed ^ 0x84407db3, c->vegetationConcentration);
	generateNoise(optionWorldSeed ^ 0xc2fb18f4, c->islandSizeModifier);
	generateNoise(optionWorldSeed ^ 0x1ab033cF, c->islandCountModifier);
	generateNoise(optionWorldSeed ^ 0xF79610E3, c->geoworldMap);
	generateNoise(optionWorldSeed ^ 0x36AC5890, c->heightModifier);
}

void bigchungusFree(bigchungus *c){
	for(int x=0;x<256;x++){
		for(int y=0;y<128;y++){
			for(int z=0;z<256;z++){
				chungusFree(c->chungi[x][y][z]);
			}
		}
	}
	memset(c->chungi,0,256*128*256*sizeof(chunk *));
}

chungus *bigchungusTryChungus(bigchungus *c, int x,int y,int z) {
	if(!inWorld(x,y,z)){return NULL;}
	return c->chungi[x&0xFF][y&0x7F][z&0xFF];
}

chungus *worldTryChungus(int x, int y, int z){
	if(!inWorld(x,y,z)){return NULL;}
	return bigchungusTryChungus(&world,x,y,z);
}

chungus *bigchungusGetChungus(bigchungus *c, int x,int y,int z) {
	if(!inWorld(x,y,z)){return NULL;}
	chungus *chng = c->chungi[x&0xFF][y&0x7F][z&0xFF];
	if(chng == NULL){
		chng = c->chungi[x&0xFF][y&0x7F][z&0xFF] = chungusNew(x,y,z);
		chungusWorldGenLoad(chng);
	}
	return chng;
}

void bigchungusFreeChungus(bigchungus *c, int x, int y, int z){
	chungus *chng = c->chungi[x&0xFF][y&0x7F][z&0xFF];
	if(chng != NULL){
		chungusFree(chng);
	}
	c->chungi[x&0xFF][y&0x7F][z&0xFF] = NULL;
}

chunk *bigchungusGetChunk(bigchungus *c, int x, int y, int z){
	if(!inWorld(x,y,z)){return NULL;}
	chungus *chng = bigchungusGetChungus(c,(x>>8)&0xFF,(y>>8)&0xFF,(z>>8)&0xFF);
	if(chng == NULL){return NULL;}
	chunk *chnk = chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
	return chnk;
}

chunk *bigchungusTryChunk(bigchungus *c, int x, int y, int z){
	if(!inWorld(x,y,z)){return NULL;}
	chungus *chng = bigchungusTryChungus(c,(x>>8)&0xFF,(y>>8)&0xFF,(z>>8)&0xFF);
	if(chng == NULL){return NULL;}
	return chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
}

u8 bigchungusGetB(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	if(!inWorld(x,y,z)){return 0;}
	chng = bigchungusGetChungus(c,x>>8,y>>8,z>>8);
	if(chng == NULL){ return 0; }
	return chungusGetB(chng,x&0xFF,y&0xFF,z&0xFF);
}

bool bigchungusGetHighestP(bigchungus *c, int x,int *retY, int z) {
	chungus *chng;

	if(x < 0){return 0;}
	if(z < 0){return 0;}
	int cx = x/CHUNGUS_SIZE;
	int cz = z/CHUNGUS_SIZE;
	if(cx >= 256){return 0;}
	if(cz >= 256){return 0;}
	x = x - (cx*CHUNGUS_SIZE);
	z = z - (cz*CHUNGUS_SIZE);

	for(int cy=127;cy >= 0;cy--){
		chng = c->chungi[cx][cy][cz];
		if(chng == NULL){
			chng = c->chungi[cx][cy][cz] = chungusNew(cx,cy,cz);
			chungusWorldGenLoad(chng);
		}
		int y;
		if(chungusGetHighestP(chng,x,&y,z)){
			*retY = y+(cy*CHUNGUS_SIZE);
			return true;
		}
	}
	return false;
}

bool bigchungusSetB(bigchungus *c, int x,int y,int z,u8 block){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng == NULL){
		chng = c->chungi[cx][cy][cz] = chungusNew(cx,cy,cz);
		chungusWorldGenLoad(chng);
		return true;
	}
	chungusSetB(chng,x&0xFF,y&0xFF,z&0xFF,block);
	return false;
}

void bigchungusBox(bigchungus *c, int x,int y,int z, int w,int h,int d,u8 block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				bigchungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void bigchungusBoxSphere(bigchungus *c, int x,int y,int z, int r, u8 block){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
		for(int cy=-r;cy<=r;cy++){
			for(int cz=-r;cz<=r;cz++){
				const int d = (cx*cx)+(cy*cy)+(cz*cz);
				if(d >= md){continue;}
				bigchungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void bigchungusBoxMine(bigchungus *c, int x,int y,int z, int w,int h,int d){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				u8 b = bigchungusGetB(c,cx+x,cy+y,cz+z);
				if(b==0){continue;}
				blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
				bigchungusSetB(c,cx+x,cy+y,cz+z,0);
			}
		}
	}
}

void bigchungusBoxMineSphere(bigchungus *c, int x,int y,int z, int r){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
		for(int cy=-r;cy<=r;cy++){
			for(int cz=-r;cz<=r;cz++){
				u8 b = bigchungusGetB(c,cx+x,cy+y,cz+z);
				if(b==0){continue;}
				const int d = (cx*cx)+(cy*cy)+(cz*cz);
				if(d >= md){continue;}
				blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
				bigchungusSetB(c,cx+x,cy+y,cz+z,0);
			}
		}
	}
}

int bigchungusTrySpawn(bigchungus *c, const ivec s){
	return((bigchungusGetB(c,s.x,s.y  ,s.z)!=0) &&
	       (bigchungusGetB(c,s.x,s.y+1,s.z)==0) &&
	       (bigchungusGetB(c,s.x,s.y+2,s.z)==0));
}

void bigchungusDetermineSpawn(bigchungus *c, const ivec s){
	const ivec sp = ivecAndS(s,~0xFF);
	for(int step = CHUNGUS_SIZE; step >= 1;step/=2){
		for(int x = step/2;x<CHUNGUS_SIZE;x+=step){
			for(int y = step/2;y<CHUNGUS_SIZE;y+=step){
				for(int z = step/2;z<CHUNGUS_SIZE;z+=step){
					const ivec cp = ivecOr(sp,ivecNew(x,y,z));
					if(bigchungusTrySpawn(c,cp)){
						c->spawn = cp;
						return;
					}
				}
			}
		}
	}
}

void bigchungusGenSpawn(bigchungus *c){
	for(int x=127;x<=129;x++){
		for(int y=1;y<=3;y++){
			for(int z=127;z<=129;z++){
				if(c->chungi[x][y][z] == NULL){
					c->chungi[x][y][z] = chungusNew(x,y,z);
					chungusWorldGenLoad(c->chungi[x][y][z]);
				}
				if(c->chungi[x][y][z]->spawn.x >= 0){
					c->spawn = ivecAdd(c->chungi[x][y][z]->spawn,ivecMulS(ivecNew(x,y,z),CHUNGUS_SIZE));
				}
			}
		}
	}
	if(!bigchungusTrySpawn(c,c->spawn)){
		bigchungusDetermineSpawn(c,c->spawn);
	}
}

void bigchungusGenHugeSpawn(bigchungus *c){
	for(int x=127;x<=129;x++){
		for(int y=1;y<=8;y++){
			for(int z=127;z<=129;z++){
				if(c->chungi[x][y][z] == NULL){
					c->chungi[x][y][z] = chungusNew(x,y,z);
					chungusWorldGenLoad(c->chungi[x][y][z]);
				}
				if(c->chungi[x][y][z]->spawn.x >= 0){
					c->spawn = ivecAdd(c->chungi[x][y][z]->spawn,ivecMulS(ivecNew(x,y,z),CHUNGUS_SIZE));
				}
			}
		}
	}
	if(!bigchungusTrySpawn(c,c->spawn)){
		bigchungusDetermineSpawn(c,c->spawn);
	}
}

ivec bigchungusGetSpawnPos(bigchungus *c){
	if(!bigchungusTrySpawn(c,c->spawn)){
		bigchungusDetermineSpawn(c,c->spawn);
	}
	return c->spawn;
}

void bigchungusUpdateClient(bigchungus *c, int p){
	character *chara = clients[p].c;
	if(chara == NULL){return;}
	int cx = ((int)chara->pos.x) >> 8;
	int cy = ((int)chara->pos.y) >> 8;
	int cz = ((int)chara->pos.z) >> 8;

	if((cx >= 0) && (cx < 256) && (cy >= 0) && (cy < 128) && (cz >= 0) && (cz < 256)){
		chungusUpdateClient(c->chungi[cx][cy][cz],p);
	}
	for(int ix=0;ix < 12; ix++){
		for(int iy=0;iy < 12; iy++){
			for(int iz=0;iz < 12; iz++){

				int ox = ix >> 1;
				if(ix & 1){ox = -ox;}
				ox = cx+ox;
				if(ox <=  0){goto xcontinue;}
				if(ox > 255){goto xcontinue;}

				int oy = iy >> 1;
				if(iy & 1){oy = -oy;}
				oy = cy+oy;
				if(oy <=  0){goto ycontinue;}
				if(oy > 127){goto ycontinue;}

				int oz = iz >> 1;
				if(iz & 1){oz = -oz;}
				oz = cz+oz;
				if(oz <=  0){continue;}
				if(oz > 255){continue;}

				chungusUpdateClient(c->chungi[ox][oy][oz],p);
			}
			ycontinue: (void)c;
		}
		xcontinue: (void)c;
	}
}

void bigchungusUnsubscribeClient(bigchungus *c, int p){
	for(int ox=0;ox < 256; ox++){
		for(int oy=0;oy < 128; oy++){
			for(int oz=0;oz < 256; oz++){
				chungus *chng = c->chungi[ox][oy][oz];
				if(chng == NULL){continue;}
				chungusUnsubscribePlayer(chng,p);
			}
		}
	}
}

void bigchungusDirtyChunk(bigchungus *c, int x, int y, int z, int client){
	chungus *chng = bigchungusTryChungus(c,(x>>8)&0xFF,(y>>8)&0xFF,(z>>8)&0xFF);
	if(chng == NULL){return;}
	chunk *chnk = chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
	if(chnk == NULL){return;}
	chunkUnsetUpdated(chnk,client);
	chungusUnsetUpdated(chng,client);
	chungusUpdateClient(chng,client);
}

void worldBox(int x, int y,int z, int w,int h,int d,u8 block){
	bigchungusBox(&world,x,y,z,w,h,d,block);
}
void worldBoxSphere(int x, int y,int z, int r,u8 block){
	bigchungusBoxSphere(&world,x,y,z,r,block);
}
u8 worldGetB(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z);
}
chungus* worldGetChungus(int x, int y, int z){
	return bigchungusGetChungus(&world,x,y,z);
}
chunk* worldGetChunk(int x, int y, int z){
	return bigchungusGetChunk(&world,x,y,z);
}
bool worldSetB(int x, int y, int z, u8 block){
	return bigchungusSetB(&world,x,y,z,block);
}
void worldDirtyChunk(int x, int y, int z, int c){
	bigchungusDirtyChunk(&world, x, y, z, c);
}
int checkCollision(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z) != 0;
}
void worldBoxMine(int x, int y, int z, int w,int h,int d){
	bigchungusBoxMine(&world,x,y,z,w,h,d);
}
void worldBoxMineSphere(int x, int y, int z, int r){
	bigchungusBoxMineSphere(&world,x,y,z,r);
}
ivec worldGetSpawnPos(){
	return bigchungusGetSpawnPos(&world);
}
void worldSetAllUpdated(){
	for(int ox=0;ox < 256; ox++){
		for(int oy=0;oy < 128; oy++){
			for(int oz=0;oz < 256; oz++){
				chungus *chng = world.chungi[ox][oy][oz];
				if(chng == NULL){continue;}
				chng->clientsUpdated = 0;
				chungusSetAllUpdated(chng, 0);
			}
		}
	}
}
