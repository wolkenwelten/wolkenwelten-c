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

#include "../sdl/sdl.h"
#include "../game/animal.h"
#include "../game/blockType.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/projectile.h"
#include "../game/throwable.h"
#include "../game/weather/weather.h"
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
#include "../voxel/meshgen/block.h"
#include "../voxel/meshgen/fluid.h"
#include "../../../common/src/game/chunkOverlay.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/game/item.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

chungus *world[256][128][256];

void worldInit(){
	chungusInit();
	chunkInit();
	animalInit();
	itemDropInit();
	projectileInit();
	throwableInit();
}

static int quicksortQueuePart(queueEntry *a, int lo, int hi){
	float p = a[hi].priority;
	int i = lo;
	for(int j = lo;j<=hi;j++){
		if(a[j].priority < p){
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
	if((x|y|z)&(~0xFFFF)){return NULL;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	return chng ? &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF] : NULL;
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

blockId worldGetB(int x,int y,int z) {
	if((x|y|z)&(~0xFFFF)){return 0;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){ return 0; }
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->block == NULL){ return 0; }
	return chnk->block->data[x&0xF][y&0xF][z&0xF];
}

blockId worldTryB(int x,int y,int z) {
	return worldGetB(x,y,z);
}

bool worldSetB(int x,int y,int z,blockId block){
	if((x|y|z)&(~0xFFFF)){return NULL;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){return false;}
	chungusSetB(chng,x,y,z,block);
	return true;
}

u8 worldGetFluid(int x,int y,int z) {
	if((x|y|z)&(~0xFFFF)){return 0;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){ return 0; }
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->fluid == NULL){ return 0; }
	return chnk->fluid->data[x&0xF][y&0xF][z&0xF];
}

u8 worldTryFluid(int x,int y,int z) {
	return worldGetFluid(x,y,z);
}

bool worldSetFluid(int x,int y,int z, u8 level){
	if((x|y|z)&(~0xFFFF)){return NULL;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){return false;}
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->fluid == NULL){chnk->fluid = chunkOverlayAllocate();}
	chnk->fluid->data[x&0xF][y&0xF][z&0xF] = level;
	return true;
}

static void worldQueueLoad(queueEntry *loadQueue, int loadQueueLen){
	if(loadQueueLen <= 0){return;}
	quicksortQueue(loadQueue,0,loadQueueLen-1);
	for(int i=loadQueueLen-1;i>=0;i--){
		chungus *chng = loadQueue[i].chng;
		msgRequestChungus(-1, chng->x,chng->y,chng->z);
	}
}

static void worldQueueDraw(queueEntry *drawQueue, int drawQueueLen){
	quicksortQueue(drawQueue,0,drawQueueLen-1);

	gfxGroupStart("World geometry");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	chunkDrawBlockQueue(drawQueue,drawQueueLen);
	chunkDrawFluidQueue(drawQueue,drawQueueLen);

	gfxGroupEnd();
}

static int queueEntryAdd(queueEntry *q, int len, int size, float priority, chunk *c){
	for(int i=0;i<len;i++){
		if(q[i].priority > priority){
			for(int ii=size-1;ii > i;ii--){q[ii] = q[ii-1];}
			q[i].chnk = c;
			q[i].priority = priority;
			return MIN(size,len + 1);
		}
	}
	if(len < size){
		q[len].chnk = c;
		q[len].priority = priority;
		return len+1;
	}else{
		return len;
	}
}

void worldQueueGenerate(const queueEntry *drawQueue, int drawQueueLen){
	#if defined(__x86_64__) || defined(__APPLE__)
	queueEntry generatorQueue[24];
	#else
	queueEntry generatorQueue[16];
	#endif
	int generatorQueueLen = 0;

	for(int i=0;i<drawQueueLen;i++){
		chunk *c = drawQueue[i].chnk;
		if(!(c->flags & CHUNK_MASK_DIRTY)){continue;}
		const float priority = drawQueue[i].priority - c->framesSkipped * 16.f;
		generatorQueueLen = queueEntryAdd(generatorQueue, generatorQueueLen, countof(generatorQueue), priority, c);
		c->framesSkipped = MIN(32,c->framesSkipped+1);
	}
	for(int i=0;i<generatorQueueLen; i++){
		//printf("Priority: %f\n", generatorQueue[i].priority);
		chunk *c = generatorQueue[i].chnk;
		chunkGenBlockMesh(c);
		chunkGenFluidMesh(c);
	}
}

void worldDraw(const character *cam){
	static queueEntry drawQueue[8192*4];
	static queueEntry loadQueue[1<<8];
	int drawQueueLen=0,loadQueueLen=0;
	if(connectionState < 2){return;}

	extractFrustum();
	// Use the subBlock view matrix in order to render chunks relative to the player.
	// This allows for maximum rendering floating point precision to avoid shaky surfaces
	// and visible block seams.
	matMul(matMVP,matSubBlockView,matProjection);

	const int dist   = (int)ceilf(renderDistance / CHUNGUS_SIZE)+1;
	const u64 cTicks = getTicks();

	const int camCX  = (int)cam->pos.x >> 8;
	const int camCY  = (int)cam->pos.y >> 8;
	const int camCZ  = (int)cam->pos.z >> 8;

	const int minCX  = MAX(  0,camCX - dist);
	const int minCY  = MAX(  0,camCY - dist);
	const int minCZ  = MAX(  0,camCZ - dist);

	const int maxCX  = MIN(256,camCX + dist);
	const int maxCY  = MIN(128,camCY + dist);
	const int maxCZ  = MIN(256,camCZ + dist);

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
		if((d < (renderDistance+CHUNGUS_SIZE)) && (CubeInFrustum(vecMulS(pos, CHUNGUS_SIZE), CHUNGUS_SIZE))){
			if(world[x][y][z] == NULL){
				world[x][y][z] = chungusNew(x,y,z);
				world[x][y][z]->requested = cTicks;
				loadQueue[loadQueueLen].priority = d;
				loadQueue[loadQueueLen++].chng   = world[x][y][z];
			}else if(world[x][y][z]->requested == 0){
				chungusQueueDraws(world[x][y][z],cam,drawQueue,&drawQueueLen);
			}else if((world[x][y][z]->requested + 300) < cTicks){
				world[x][y][z]->requested = cTicks + rngValA(255);
				loadQueue[loadQueueLen].priority = d;
				loadQueue[loadQueueLen++].chng   = world[x][y][z];
			}
		}
	}
	}
	}
	worldQueueLoad(loadQueue, loadQueueLen);
	worldQueueGenerate(drawQueue, drawQueueLen);
	worldQueueDraw(drawQueue, drawQueueLen);
}

void worldBox(int x,int y,int z, int w,int h,int d,blockId block){
	for(int cx=0;cx<w;cx++){
	for(int cy=0;cy<h;cy++){
	for(int cz=0;cz<d;cz++){
		worldSetB(cx+x,cy+y,cz+z,block);
	}
	}
	}
}

void worldFreeFarChungi(const character *cam){
	if(cam == NULL){
		worldInit();
		return;
	}
	const int len = chungusGetActiveCount();
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
			msgUnsubChungus(-1, x,y,z);
		}
	}
}

void worldBoxSphere(int x,int y,int z, int r, blockId block){
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

void worldMine(int x, int y, int z){
	const blockId b = worldGetB(x,y,z);
	msgMineBlock(x,y,z,b,0);
	if((b == I_Grass) || (b == I_Dry_Grass)){
		worldSetB(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
	}
}

void worldBreak(int x, int y, int z){
	const blockId b = worldGetB(x,y,z);
	msgMineBlock(x,y,z,b,1);
	if((b == I_Grass) || (b == I_Dry_Grass)){
		worldSetB(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
	}
}

void worldBoxMine(int x, int y, int z, int w, int h, int d){
	for(int cx=0;cx<w;cx++){
	for(int cy=0;cy<h;cy++){
	for(int cz=0;cz<d;cz++){
		worldMine(x+cx,y+cy,z+cz);
	}
	}
	}
}

bool worldIsLoaded(int x, int y, int z){
	return worldTryChungus(x>>8,y>>8,z>>8) != NULL;
}

void worldSetChunkUpdated(int x, int y, int z){
	chunk *chnk = worldGetChunk(x,y,z);
	if(chnk == NULL){return;}
	chnk->flags |= CHUNK_FLAG_DIRTY;
}

bool worldShouldBeLoaded(const vec cpos){
	(void)cpos;
	return true;
}

u8 worldGetFire(int x,int y,int z) {
	if((x|y|z)&(~0xFFFF)){return 0;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){ return 0; }
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->flame == NULL){ return 0; }
	return chnk->flame->data[x&0xF][y&0xF][z&0xF];
}

u8 worldTryFire(int x,int y,int z) {
	return worldGetFire(x,y,z);
}

u8 worldTryLight(int x, int y, int z){
	if((x|y|z)&(~0xFFFF)){return gtimeGetBlockBrightness(gtimeGetTimeOfDay());}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){ return gtimeGetBlockBrightness(gtimeGetTimeOfDay()); }
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->light == NULL){ return gtimeGetBlockBrightness(gtimeGetTimeOfDay()); }
	return chnk->light->data[x&0xF][y&0xF][z&0xF];
}

bool worldSetFire(int x,int y,int z, u8 level){
	if((x|y|z)&(~0xFFFF)){return NULL;}
	chungus *chng = world[x>>8][(y>>8)&0x7F][z>>8];
	if(chng == NULL){return false;}
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	if(chnk->flame == NULL){chnk->flame = chunkOverlayAllocate();}
	chnk->flame->data[x&0xF][y&0xF][z&0xF] = level;
	return true;
}

void chungusRecvUnsub(const packet *p){
	const int cx = p->v.u8[0];
	const int cy = p->v.u8[1];
	const int cz = p->v.u8[2];

	chungusFree(worldGetChungus(cx,cy,cz));
}
