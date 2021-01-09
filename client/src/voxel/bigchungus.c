#include "bigchungus.h"

#include "../sdl/sdl.h"
#include "../game/blockType.h"
#include "../game/weather.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
#include "../gfx/texture.h"
#include "../network/client.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

chungus *world[256][128][256];

static int quicksortQueuePart(queueEntry *a, int lo, int hi){
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

static void quicksortQueue(queueEntry *a, int lo, int hi){
	if(lo >= hi){ return; }
	int p = quicksortQueuePart(a,lo,hi);
	quicksortQueue        (a, lo , p-1);
	quicksortQueue        (a, p+1, hi);
}

static bool chungusInFrustum(const vec pos) {
	return CubeInFrustum(vecMulS(pos,CHUNGUS_SIZE),CHUNGUS_SIZE);
}

static float chungusDistance(const vec cam, const vec pos) {
	const vec np = vecAddS(vecMulS(pos,CHUNGUS_SIZE),CHUNGUS_SIZE/2);
	return vecMag(vecSub(cam,np));
}

void worldFree(){
	for(int x=0;x<256;x++){
		for(int y=0;y<128;y++){
			for(int z=0;z<256;z++){
				if(world[x][y][z] == NULL){continue;}
				chungusFree(world[x][y][z]);
			}
		}
	}
	memset(world,0,256*128*256*sizeof(chunk *));
}

chungus *worldTryChungus(int x,int y,int z){
	if((x|y|z)&(~0xFF)){return NULL;}
	return world[x&0xFF][y&0x7F][z&0xFF];
}

chunk *worldTryChunk(int x, int y, int z){
	chungus *chng = worldTryChungus(x,y,z);
	if(chng == NULL){return NULL;}
	return chungusGetChunk(chng,x,y,z);
}

chungus *worldGetChungus(int x,int y,int z){
	if((x|y|z)&(~0xFF)){return NULL;}
	return world[x&0xFF][y&0x7F][z&0xFF];
}

chunk *worldGetChunk(int x, int y, int z){
	if((x|y|z)&(~0xFFFF)){return NULL;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){return NULL;}
	chunk *chnk = chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
	return chnk;
}

u8 worldTryB(int x,int y,int z) {
	if((x|y|z)&(~0xFFFF)){return 0;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){ return 0; }
	return chungusGetB(chng,x,y,z);
}

u8 worldGetB(int x,int y,int z) {
	if((x|y|z)&(~0xFFFF)){return 0;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){ return 0; }
	chunk *chnk = chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk == NULL){ return 0; }
	return chnk->data[x&0xF][y&0xF][z&0xF];
}

bool worldSetB(int x,int y,int z,u8 block){
	if((x|y|z)&(~0xFFFF)){return NULL;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){return false;}
	chungusSetB(chng,x,y,z,block);
	return true;
}

void worldDraw(const character *cam){
	static queueEntry drawQueue[8192*4];
	static queueEntry loadQueue[1<<9];
	int drawQueueLen=0,loadQueueLen=0;
	if(connectionState < 2){return;}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);
	textureBind(tBlocksArr);
	extractFrustum();

	shaderBind(sBlockMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sBlockMesh,matMVP);
	shaderBrightness(sBlockMesh,skyBrightness);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	const int camCX  = (int)cam->pos.x >> 8;
	const int camCY  = (int)cam->pos.y >> 8;
	const int camCZ  = (int)cam->pos.z >> 8;
	const int dist   = (int)ceilf(renderDistance / CHUNGUS_SIZE)+1;
	const int minCX  = MAX(  0,camCX - dist);
	const int minCY  = MAX(  0,camCY - dist);
	const int minCZ  = MAX(  0,camCZ - dist);
	const int maxCX  = MIN(256,camCX + dist);
	const int maxCY  = MIN(128,camCY + dist);
	const int maxCZ  = MIN(256,camCZ + dist);
	const u64 cTicks = getTicks();

	for(int x=camCX-dist;x<camCX+dist;x++){
	for(int y=camCY-dist;y<camCY+dist;y++){
	for(int z=camCZ-dist;z<camCZ+dist;z++){
		cloudsDraw(x,y,z);
	}
	}
	}
	if(cam->flags & CHAR_SPAWNING){return;}

	for(int x=minCX;x<maxCX;x++){
	for(int y=minCY;y<maxCY;y++){
	for(int z=minCZ;z<maxCZ;z++){
		const vec pos = vecNew(x,y,z);
		float d = chungusDistance(cam->pos,pos);
		if((d < (renderDistance+CHUNGUS_SIZE)) && (chungusInFrustum(pos))){
			if(world[x][y][z] == NULL){
				world[x][y][z] = chungusNew(x,y,z);
				world[x][y][z]->requested = cTicks;
				loadQueue[loadQueueLen].distance = d;
				loadQueue[loadQueueLen++].chng   = world[x][y][z];
			}else if(world[x][y][z]->requested == 0){
				chungusQueueDraws(world[x][y][z],cam,drawQueue,&drawQueueLen);
			}else if((world[x][y][z]->requested + 3000) < cTicks){
				world[x][y][z]->requested = cTicks;
				loadQueue[loadQueueLen].distance = d;
				loadQueue[loadQueueLen++].chng   = world[x][y][z];
			}
		}
	}
	}
	}
	if(loadQueueLen > 0){
		quicksortQueue(loadQueue,0,loadQueueLen-1);
		for(int i=loadQueueLen-1;i>=0;i--){
			chungus *chng = loadQueue[i].chng;
			msgRequestChungus(chng->x,chng->y,chng->z);
		}
	}

	quicksortQueue(drawQueue,0,drawQueueLen-1);
	for(int i=0;i<drawQueueLen;i++){
		chunkDraw(drawQueue[i].chnk,drawQueue[i].distance);
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void worldBox(int x,int y,int z, int w,int h,int d,u8 block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				worldSetB(cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void worldFreeFarChungi(const character *cam){
	int len = chungusGetActiveCount();
	for(int i=0;i<len;i++){
		chungus *chng = chungusGetActive(i);
		if(chng->nextFree != NULL){continue;}
		int x = (int)chng->x;
		int y = (int)chng->y;
		int z = (int)chng->z;
		float d = chungusDistance(cam->pos,vecNew(x,y,z));
		if(d > (renderDistance + 256.f)){
			chungusFree(world[x][y][z]);
			world[x][y][z] = NULL;
			msgUnsubChungus(x,y,z);
		}
	}
}

void worldBoxSphere(int x,int y,int z, int r, u8 block){
	const int md = r*r;
	for(int cx=-r;cx<=r;cx++){
		for(int cy=-r;cy<=r;cy++){
			for(int cz=-r;cz<=r;cz++){
				const int d = (cx*cx)+(cy*cy)+(cz*cz);
				if(d >= md){continue;}
				worldSetB(cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void worldBoxSphereDirty(int x,int y,int z, int r){
	int xs = (x-r)>>4;
	int xe = (x+r)>>4;
	if(xe==xs){xe++;}
	int ys = (y-r)>>4;
	int ye = (y+r)>>4;
	if(ye==ys){ye++;}
	int zs = (z-r)>>4;
	int ze = (z+r)>>4;
	if(ze==zs){ze++;}

	for(int cx=xs;cx<=xe;cx++){
		for(int cy=ys;cy<=ye;cy++){
			for(int cz=zs;cz<=ze;cz++){
				msgDirtyChunk(cx<<4,cy<<4,cz<<4);
			}
		}
	}
}

void worldSetChungusLoaded(int x, int y, int z){
	chungus *chng = world[x&0xFF][y&0x7F][z&0xFF];
	if(chng != NULL){chng->requested = 0;}
}

int checkCollision(int x, int y, int z){
	return worldGetB(x,y,z);
}

void worldBoxMine(int x, int y, int z, int w, int h, int d){
	worldBox(x,y,z,w,h,d,0);
}

bool worldIsLoaded(int x, int y, int z){
	return worldTryChungus(x>>8,y>>8,z>>8) != NULL;
}
