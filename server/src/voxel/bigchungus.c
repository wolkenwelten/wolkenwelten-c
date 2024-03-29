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
	return bigchungusTryChungus(&world,x,y,z);
}

chungus *bigchungusGetChungus(bigchungus *c, int x,int y,int z) {
	if(!inWorld(x,y,z)){return NULL;}
	chungus *chng = c->chungi[x&0xFF][y&0x7F][z&0xFF];
	if(chng == NULL){
		chng = c->chungi[x&0xFF][y&0x7F][z&0xFF] = chungusNew(x,y,z);
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

chunk *worldTryChunk(int x, int y, int z){
	return bigchungusTryChunk(&world,x,y,z);
}

blockId bigchungusGetB(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	if(!inWorld(x,y,z)){return 0;}
	chng = bigchungusGetChungus(c,x>>8,y>>8,z>>8);
	if(chng == NULL){ return 0; }
	return chungusGetB(chng,x&0xFF,y&0xFF,z&0xFF);
}

blockId bigchungusTryB(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	if(!inWorld(x,y,z)){return 0;}
	chng = bigchungusTryChungus(c,x>>8,y>>8,z>>8);
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
		}
		int y;
		if(chungusGetHighestP(chng,x,&y,z)){
			*retY = y+(cy*CHUNGUS_SIZE);
			return true;
		}
	}
	return false;
}

bool bigchungusSetB(bigchungus *c, int x,int y,int z,blockId block){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng == NULL){
		chng = c->chungi[cx][cy][cz] = chungusNew(cx,cy,cz);
		return true;
	}
	chungusSetB(chng,x&0xFF,y&0xFF,z&0xFF,block);
	return false;
}

void bigchungusBox(bigchungus *c, int x,int y,int z, int w,int h,int d,blockId block){
	for(int cx=0;cx<w;cx++){
	for(int cy=0;cy<h;cy++){
	for(int cz=0;cz<d;cz++){
		bigchungusSetB(c,cx+x,cy+y,cz+z,block);
	}
	}
	}
}

void bigchungusBoxSphere(bigchungus *c, int x,int y,int z, int r, blockId block){
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

void bigchungusMine(bigchungus *c, int x,int y,int z){
	(void)c;
	blockMiningMineBlock(x,y,z,0);
}

void bigchungusBreak(bigchungus *c, int x,int y,int z){
	(void)c;
	blockMiningMineBlock(x,y,z,1);
}

void bigchungusBoxMine(bigchungus *c, int x,int y,int z, int w,int h,int d){
	for(int cx=0;cx<w;cx++){
	for(int cy=0;cy<h;cy++){
	for(int cz=0;cz<d;cz++){
		blockId b = bigchungusGetB(c,cx+x,cy+y,cz+z);
		if(b==0){continue;}
		bigchungusSetB(c,cx+x,cy+y,cz+z,0);
		blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
	}
	}
	}
}

void bigchungusBoxMineSphere(bigchungus *c, int x,int y,int z, int r){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
	for(int cy=-r;cy<=r;cy++){
	for(int cz=-r;cz<=r;cz++){
		blockId b = bigchungusGetB(c,cx+x,cy+y,cz+z);
		if(b==0){continue;}
		const int d = (cx*cx)+(cy*cy)+(cz*cz);
		if(d >= md){continue;}
		bigchungusSetB(c,cx+x,cy+y,cz+z,0);
		blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
	}
	}
	}
}

static int bigchungusTrySpawn(bigchungus *c, int sx, int sy, int sz){
	return((bigchungusGetB(c,sx,sy  ,sz)!=0) &&
	       (bigchungusGetB(c,sx,sy+1,sz)==0) &&
	       (bigchungusGetB(c,sx,sy+2,sz)==0));
}

static void bigchungusDetermineSpawn(bigchungus *c, int sx, int sy, int sz){
	const int spx = sx & ~0xFF;
	const int spy = sy & ~0xFF;
	const int spz = sz & ~0xFF;
	for(int step = CHUNGUS_SIZE; step >= 1;step/=2){
		for(int x = step/2;x<CHUNGUS_SIZE;x+=step){
		for(int y = step/2;y<CHUNGUS_SIZE;y+=step){
		for(int z = step/2;z<CHUNGUS_SIZE;z+=step){
			const int cpx = spx | x;
			const int cpy = spy | y;
			const int cpz = spz | z;
			if(bigchungusTrySpawn(c,cpx,cpy,cpz)){
				c->sx = cpx;
				c->sy = cpy;
				c->sz = cpz;
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
		chungus *chng = c->chungi[x][y][z];
		if(chng == NULL){
			chng = c->chungi[x][y][z] = chungusNew(x,y,z);
		}
		if(chng->sx | chng->sy | chng->sz){
			c->sx = chng->sx | (x << 8);
			c->sy = chng->sy | (y << 8);
			c->sz = chng->sz | (z << 8);
		}
	}
	}
	}
	if(!bigchungusTrySpawn(c,c->sx,c->sy,c->sz)){
		bigchungusDetermineSpawn(c,c->sx,c->sy,c->sz);
	}
}

void bigchungusGenHugeSpawn(bigchungus *c){
	for(int x=126;x<=130;x++){
	for(int y=1;y<=32;y++){
	for(int z=126;z<=130;z++){
		chungus *chng = c->chungi[x][y][z];
		if(chng == NULL){
			chng = c->chungi[x][y][z] = chungusNew(x,y,z);
		}
		if(chng->sx | chng->sy | chng->sz){
			c->sx = chng->sx | (x << 8);
			c->sy = chng->sy | (y << 8);
			c->sz = chng->sz | (z << 8);
		}
	}
	}
	}
	if(!bigchungusTrySpawn(c,c->sx,c->sy,c->sz)){
		bigchungusDetermineSpawn(c,c->sx,c->sy,c->sz);
	}
}

vec bigchungusGetSpawnPos(bigchungus *c){
	/*
	if(!bigchungusTrySpawn(c,c->spawn)){
		bigchungusDetermineSpawn(c,c->spawn);
	}*/
	return vecNew(c->sx,c->sy,c->sz);
}

void bigchungusSetSpawnPos(bigchungus *c, vec pos){
	c->sx = pos.x;
	c->sy = pos.y;
	c->sz = pos.z;
}

void bigchungusUpdateClient(bigchungus *c, int p){
	character *chara = clients[p].c;
	if(chara == NULL){return;}
	clients[p].chnkUpdateIter++;
	//printf("bigchungusUpdateClient %u\n",clients[p].chnkUpdateIter);
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

void worldBox(int x, int y,int z, int w,int h,int d,blockId block){
	bigchungusBox(&world,x,y,z,w,h,d,block);
}
void worldBoxSphere(int x, int y,int z, int r,blockId block){
	bigchungusBoxSphere(&world,x,y,z,r,block);
}
blockId worldGetB(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z);
}
blockId worldTryB(int x, int y, int z){
	return bigchungusTryB(&world,x,y,z);
}
chungus* worldGetChungus(int x, int y, int z){
	return bigchungusGetChungus(&world,x,y,z);
}
chunk* worldGetChunk(int x, int y, int z){
	return bigchungusGetChunk(&world,x,y,z);
}
bool worldSetB(int x, int y, int z, blockId block){
	return bigchungusSetB(&world,x,y,z,block);
}
void worldDirtyChunk(int x, int y, int z, int c){
	bigchungusDirtyChunk(&world, x, y, z, c);
}
int checkCollision(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z) != 0;
}
void worldMine(int x, int y, int z){
	bigchungusMine(&world,x,y,z);
}
void worldBreak(int x, int y, int z){
	bigchungusBreak(&world,x,y,z);
}
void worldBoxMine(int x, int y, int z, int w,int h,int d){
	bigchungusBoxMine(&world,x,y,z,w,h,d);
}
void worldBoxMineSphere(int x, int y, int z, int r){
	bigchungusBoxMineSphere(&world,x,y,z,r);
}
vec worldGetSpawnPos(){
	return bigchungusGetSpawnPos(&world);
}
void worldSetSpawnPos(vec pos){
	bigchungusSetSpawnPos(&world,pos);
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

chungus* worldTryChungusV(const vec pos){
	const int cx = ((uint)pos.x) >> 8;
	const int cy = ((uint)pos.y) >> 8;
	const int cz = ((uint)pos.z) >> 8;
	return worldTryChungus(cx,cy,cz);
}

bool worldIsLoaded(int x, int y, int z){
	return worldTryChungus(x>>8,y>>8,z>>8) != NULL;
}

bool worldShouldBeLoaded(const vec cpos){
	if(worldIsLoaded(cpos.x,cpos.y,cpos.z)){return true;}

	for(uint ii=0;ii<clientCount;++ii){
		if(clients[ii].state)    {continue;}
		if(clients[ii].c == NULL){continue;}
		const float cdist = vecMag(vecSub(cpos,clients[ii].c->pos));
		if(cdist < 1024.f){return true;}
	}
	return false;
}

bool bigchungusSetFluid(bigchungus *c, int x, int y, int z, u8 level){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng == NULL){return false;}
	chungusSetFluid(chng, x&0xFF, y&0xFF, z&0xFF, level);
	return true;
}

u8 bigchungusTryFluid(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	if(!inWorld(x,y,z)){return 0;}
	chng = bigchungusTryChungus(c,x>>8,y>>8,z>>8);
	if(chng == NULL){ return 0; }
	return chungusGetFluid(chng,x&0xFF,y&0xFF,z&0xFF);
}

u8 bigchungusGetFluid(bigchungus *c, int x,int y,int z) {
	return bigchungusTryFluid(c,x,y,z);
}

u8 worldTryFluid(int x, int y, int z){
	return bigchungusTryFluid(&world,x,y,z);
}
u8 worldGetFluid(int x, int y, int z){
	return bigchungusGetFluid(&world,x,y,z);
}
bool worldSetFluid(int x, int y, int z, u8 level){
	return bigchungusSetFluid(&world,x,y,z, level);
}

bool bigchungusSetFire(bigchungus *c, int x, int y, int z, u8 strength){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng == NULL){return false;}
	chungusSetFire(chng, x&0xFF, y&0xFF, z&0xFF, strength);
	return true;
}

u8 bigchungusTryFire(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	if(!inWorld(x,y,z)){return 0;}
	chng = bigchungusTryChungus(c,x>>8,y>>8,z>>8);
	if(chng == NULL){ return 0; }
	return chungusGetFire(chng,x&0xFF,y&0xFF,z&0xFF);
}

u8 bigchungusGetFire(bigchungus *c, int x,int y,int z) {
	return bigchungusTryFire(c,x,y,z);
}

u8 worldTryFire(int x, int y, int z){
	return bigchungusTryFluid(&world,x,y,z);
}

u8 worldGetFire(int x, int y, int z){
	return bigchungusGetFire(&world,x,y,z);
}

bool worldSetFire(int x, int y, int z, u8 strength){
	return bigchungusSetFire(&world,x,y,z, strength);
}
