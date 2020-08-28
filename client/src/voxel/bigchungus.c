#include "bigchungus.h"

#include "../game/blockType.h"
#include "../game/entity.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../misc/options.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bigchungus world;

bool chungusInFrustum(float x,float y,float z) {
	x = x * CHUNGUS_SIZE;
	y = y * CHUNGUS_SIZE;
	z = z * CHUNGUS_SIZE;
	return CubeInFrustum(x,y,z,CHUNGUS_SIZE);
}

float chungusRoughDistance(const character *cam, float x, float y,float z) {
	x = x * CHUNGUS_SIZE + CHUNGUS_SIZE/2;
	y = y * CHUNGUS_SIZE + CHUNGUS_SIZE/2;
	z = z * CHUNGUS_SIZE + CHUNGUS_SIZE/2;

	float xdiff = fabsf(x-cam->x);
	float ydiff = fabsf(y-cam->y);
	float zdiff = fabsf(z-cam->z);

	return sqrtf((xdiff*xdiff)+(ydiff*ydiff)+(zdiff*zdiff));
}

void bigchungusInit(bigchungus *c){
	memset(c->chungi,0,256*128*256*sizeof(chunk *));
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

chunk *bigchungusGetChunk(bigchungus *c, int x, int y, int z){
	chungus *chng = bigchungusGetChungus(c,(x>>8)&0xFF,(y>>8)&0xFF,(z>>8)&0xFF);
	if(chng == NULL){return NULL;}
	chunk *chnk = chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
	return chnk;
}

chungus *bigchungusGetChungus(bigchungus *c, int x,int y,int z) {
	if((x|y|z)&(~0xFF)){return NULL;}
	chungus *chng = c->chungi[x&0xFF][y&0x7F][z&0xFF];
	return chng;
}

uint8_t bigchungusGetB(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	chng = bigchungusGetChungus(c,x/CHUNGUS_SIZE,y/CHUNGUS_SIZE,z/CHUNGUS_SIZE);
	if(chng == NULL){ return 0; }
	return chungusGetB(chng,x%CHUNGUS_SIZE,y%CHUNGUS_SIZE,z%CHUNGUS_SIZE);
}

bool bigchungusSetB(bigchungus *c, int x,int y,int z,uint8_t block){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng != NULL){
		chungusSetB(chng,x%CHUNGUS_SIZE,y%CHUNGUS_SIZE,z%CHUNGUS_SIZE,block);
		return true;
	}
	return false;
}

int quicksortQueuePart(queueEntry *a, int lo, int hi){
	float p = a[hi].distance;
	int i = lo;
	for(int j = lo;j<=hi;j++){
		if(a[j].distance < p){
			queueEntry t = a[i];
			a[i] = a[j];
			a[j] = t;
			i++;
		}
	}
	queueEntry t = a[i];
	a[i] = a[hi];
	a[hi] = t;
	return i;
}

void quicksortQueue(queueEntry *a, int lo, int hi){
	if(lo >= hi){ return; }
	int p = quicksortQueuePart(a,lo,hi);
	quicksortQueue        (a, lo , p-1);
	quicksortQueue        (a, p+1, hi);
}

void bigchungusDraw(bigchungus *c, character *cam){
	float matMVP[16];
	static queueEntry drawQueue[8192*4];
	static queueEntry loadQueue[128];
	int drawQueueLen=0,loadQueueLen=0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	textureBind(tBlocks);
	extractFrustum();

	shaderBind(sBlockMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sBlockMesh,matMVP);

	const int camCX = (int)cam->x >> 8;
	const int camCY = (int)cam->y >> 8;
	const int camCZ = (int)cam->z >> 8;
	const int dist  = (int)ceilf(CHUNK_RENDER_DISTANCE / CHUNGUS_SIZE)+1;
	const int minCX = MAX(0,camCX - dist);
	const int minCY = MAX(0,camCY - dist);
	const int minCZ = MAX(0,camCZ - dist);
	const int maxCX = MIN(255,camCX + dist);
	const int maxCY = MIN(127,camCY + dist);
	const int maxCZ = MIN(255,camCZ + dist);

	for(int x=minCX;x<maxCX;x++){
		if((x <= 0) || (x >= 255)){continue;}
		for(int y=minCY;y<maxCY;y++){
			if((y <= 0) || (y >= 127)){continue;}
			for(int z=minCZ;z<maxCZ;z++){
				if((z <= 0) || (z >= 255)){continue;}

				float d = chungusRoughDistance(cam,x,y,z);
				if((d < (CHUNK_RENDER_DISTANCE+CHUNGUS_SIZE)) && (chungusInFrustum(x,y,z))){
					if(c->chungi[x][y][z] == NULL){
						c->chungi[x][y][z] = chungusNew(x*CHUNGUS_SIZE,y*CHUNGUS_SIZE,z*CHUNGUS_SIZE);
						loadQueue[loadQueueLen].distance = d;
						loadQueue[loadQueueLen].chng     = c->chungi[x][y][z];
						++loadQueueLen;
					}
					chungusQueueDraws(c->chungi[x][y][z],cam,drawQueue,&drawQueueLen);
				}
			}
		}
	}
	if(loadQueueLen > 0){
		quicksortQueue(loadQueue,0,loadQueueLen-1);
		for(int i=0;i<loadQueueLen;i++){
			chungus *chng = loadQueue[i].chng;
			msgRequestChungus(chng->x,chng->y,chng->z);
		}
	}

	quicksortQueue(drawQueue,0,drawQueueLen-1);
	for(int i=0;i<drawQueueLen;i++){
		chunkDraw(drawQueue[i].chnk,drawQueue[i].distance);
	}
}


void bigchungusBox(bigchungus *c, int x,int y,int z, int w,int h,int d,uint8_t block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				bigchungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void bigchungusFreeFarChungi(bigchungus *c, character *cam){
	int len = chungusGetActiveCount();
	for(int i=0;i<len;i++){
		chungus *chng = chungusGetActive(i);
		if(chng->nextFree != NULL){continue;}
		int x = (int)chng->x>>8;
		int y = (int)chng->y>>8;
		int z = (int)chng->z>>8;
		float d = chungusRoughDistance(cam,x,y,z);
		if(d > (CHUNK_RENDER_DISTANCE + 256.f)){
			chungusFree(c->chungi[x][y][z]);
			c->chungi[x][y][z] = NULL;
		}
	}
}

void bigchungusBoxSphere(bigchungus *c, int x,int y,int z, int r, uint8_t block){
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

void worldBox(int x, int y,int z, int w,int h,int d,uint8_t block){
	bigchungusBox(&world,x,y,z,w,h,d,block);
}
void worldBoxSphere(int x, int y,int z, int r,uint8_t block){
	bigchungusBoxSphere(&world,x,y,z,r,block);
}
uint8_t worldGetB(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z);
}
chungus* worldGetChungus(int x, int y, int z){
	return bigchungusGetChungus(&world,x,y,z);
}
chunk* worldGetChunk(int x, int y, int z){
	return bigchungusGetChunk(&world,x,y,z);
}
bool worldSetB(int x, int y, int z, uint8_t block){
	return bigchungusSetB(&world,x,y,z,block);
}
void worldSetChungusLoaded(int x, int y, int z){
	chungus *chng = bigchungusGetChungus(&world,x>>8,y>>8,z>>8);
	if(chng != NULL){chng->loaded = 1;}
}
int checkCollision(int x, int y, int z){
	return bigchungusGetB(&world,x,y,z) != 0;
}
