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
#include "../voxel/chunk.h"

#include "../main.h"
#include "../game/blockType.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gui/gui.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
#include "../gfx/texture.h"
#include "../sdl/sdl.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunkvertbuf.h"
#include "../../../common/src/game/chunkOverlay.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "meshgen/block.h"
#include "meshgen/fluid.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define MIN_CHUNKS_GENERATED_PER_FRAME (8)
#define POS_MASK (CHUNK_SIZE-1)
#define EDGE (CHUNK_SIZE-1)


u64 chunksDirtied = 0;
u64 chunksCopied = 0;

void chunkInit(){
	chunkvertbufInit();
}

bool chunkInFrustum(const chunk *c){
	const vec pos = vecNew(c->x, c->y, c->z);
	return CubeInFrustum(pos,CHUNK_SIZE);
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	chunkvertbufFree(c);
	if(c->fluid){
		chunkOverlayFree(c->fluid);
		c->fluid = NULL;
	}
	if(c->block){
		chunkOverlayFree(c->block);
		c->block = NULL;
	}
}

void chunkReset(chunk *c, int x, int y, int z){
	c->x = x;
	c->y = y;
	c->z = z;
	c->flags = CHUNK_FLAG_DIRTY;
	c->framesSkipped = 0;
	chunkFree(c);
}

void chunkBox(chunk *c, u16 x,u16 y,u16 z,u16 gx,u16 gy,u16 gz,blockId block){
	if(c->block == NULL){c->block = chunkOverlayAllocate();}
	for(int cx=x;cx<gx;cx++){
	for(int cy=y;cy<gy;cy++){
	for(int cz=z;cz<gz;cz++){
		c->block->data[cx][cy][cz] = block;
	}
	}
	}
	c->flags |= CHUNK_FLAG_DIRTY | CHUNK_FLAG_FLUID_DIRTY;
	if(( x&EDGE) ==  0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if(( y&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if(( z&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((gx&EDGE) == EDGE){worldSetChunkUpdated(x+1,y  ,z  );}
	if((gy&EDGE) == EDGE){worldSetChunkUpdated(x  ,y+1,z  );}
	if((gz&EDGE) == EDGE){worldSetChunkUpdated(x  ,y  ,z+1);}
}

static void worldSetDirty(u16 x, u16 y, u16 z){
	chunk *chnk = worldTryChunk(x,y,z);
	if(!chnk){return;}
	chnk->flags |= CHUNK_FLAG_DIRTY;
}

void chunkSetB(chunk *c,u16 x,u16 y,u16 z,blockId block){
	if(c->block == NULL){c->block = chunkOverlayAllocate();}
	c->block->data[x&POS_MASK][y&POS_MASK][z&POS_MASK] = block;
	c->flags |= CHUNK_FLAG_DIRTY;
	for(int cx=-1;cx<2;cx++){
	for(int cy=-1;cy<2;cy++){
	for(int cz=-1;cz<2;cz++){
		//worldSetChunkUpdated(x+cx*CHUNK_SIZE,y+cy+CHUNK_SIZE,z+cz+CHUNK_SIZE);
		worldSetDirty(x+cx*CHUNK_SIZE,y+cy+CHUNK_SIZE,z+cz+CHUNK_SIZE);
	}
	}
	}
	c->flags |= CHUNK_FLAG_DIRTY | CHUNK_FLAG_FLUID_DIRTY;
	if((x&EDGE) ==  0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if((x&EDGE) == EDGE){worldSetChunkUpdated(x+1,y  ,z  );}
	if((y&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if((y&EDGE) == EDGE){worldSetChunkUpdated(x  ,y+1,z  );}
	if((z&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((z&EDGE) == EDGE){worldSetChunkUpdated(x  ,y  ,z+1);}
}

static void chunkBlockDraw(chunk *c, float d, sideMask mask){
	// Since chunk mesh generation & upload is rate limited, we might not have a vertbuf yet.
	// Trying to draw with an empty vertbuf would dereference null, avoid that.
	if(c->blockVertbuf == NULL){return;}

	float fIn = 1.f;
	if(c->fadeIn > 0){
		fIn = 1.f - ((--c->fadeIn) / (float)FADE_IN_FRAMES);
	}

	if(d > (fadeoutStartDistance)){
		shaderAlpha(sBlockMesh,(1.f-((d-(fadeoutStartDistance))/fadeoutDistance)) * fIn);
	}else{
		shaderAlpha(sBlockMesh,1.f * fIn);
	}
	shaderTransform(sBlockMesh,c->x-subBlockViewOffset.x,c->y-subBlockViewOffset.y,c->z-subBlockViewOffset.z);
	chunkvertbufDrawOne(mask, c->blockVertbuf);
}

void chunkDrawBlockQueue(const queueEntry *queue, int queueLen){
	textureBind(tBlocksArr);
	shaderBind(sBlockMesh);
	shaderMatrix(sBlockMesh,matMVP);

	for(int i=0;i<queueLen;i++){
		chunkBlockDraw(queue[i].chnk,queue[i].priority,queue[i].mask);
	}
}

static void chunkFluidDraw(chunk *c, float d, sideMask mask){
	// Since chunk mesh generation & upload is rate limited, we might not have a vertbuf yet.
	// Trying to draw with an empty vertbuf would dereference null, avoid that.
	if(c->fluidVertbuf == NULL){return;}

	const float fadeIn = 0.8f;
	if(d > (fadeoutStartDistance)){
		shaderAlpha(sFluidMesh,(1.f-((d-(fadeoutStartDistance))/fadeoutDistance) * fadeIn));
	}else{
		shaderAlpha(sFluidMesh,fadeIn);
	}
	shaderTransform(sFluidMesh,c->x-subBlockViewOffset.x,c->y-subBlockViewOffset.y,c->z-subBlockViewOffset.z);
	chunkvertbufDrawOne(mask, c->fluidVertbuf);
}

void chunkDrawFluidQueue(const queueEntry *queue, int queueLen){
	textureBind(tBlocksArr);
	shaderBind(sFluidMesh);
	shaderMatrix(sFluidMesh,matMVP);

	for(int i=0;i<queueLen;i++){
		chunkFluidDraw(queue[i].chnk, queue[i].priority, queue[i].mask);
	}
}

void chunkRecvEmpty(const packet *p){
	const u16 x = p->v.u16[0];
	const u16 y = p->v.u16[1];
	const u16 z = p->v.u16[2];
	const u16 t = p->v.u16[3];
	chungus *chng = worldGetChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){return;}
	chunk *chnk = chungusGetChunkOrNew(chng,x,y,z);
	if(chnk == NULL){return;}
	switch(t){
	default:
		break;
	case chunkOverlayBlock:
		if(chnk->block){
			chunkOverlayFree(chnk->block);
			chnk->block = NULL;
		}
		break;
	case chunkOverlayFluid:
		if(chnk->fluid){
			chunkOverlayFree(chnk->fluid);
			chnk->fluid = NULL;
		}
		break;
	case chunkOverlayFire:
		if(chnk->flame){
			chunkOverlayFree(chnk->flame);
			chnk->flame = NULL;
		}
		break;
	}
}

void chunkDirtyRegion(int cx, int cy, int cz, uint flag){
	for(int x=-1;x<2;x++){
	for(int y=-1;y<2;y++){
	for(int z=-1;z<2;z++){
		chunk *chnk = worldTryChunk(cx+(x*CHUNK_SIZE),cy+(y*CHUNK_SIZE),cz+(z*CHUNK_SIZE));
		if(!chnk){continue;}
		chnk->flags |= flag;
	}
	}
	}
}

int chunkOverlayCopyAndCompare(void *dest, const void *source){
	const u64 *from = source;
	u64 *to = dest;
	int ret = 0;
	for(uint i=0;i<(sizeof(chunkOverlay) / sizeof(u64));i++){
		if(*to != *from){ret++;}
		*to++ = *from++;
	}
	if(ret){chunksCopied++;}
	return ret;
}

void chunkRecvUpdate(const packet *p){
	const u16 x = p->v.u16[2048];
	const u16 y = p->v.u16[2049];
	const u16 z = p->v.u16[2050];
	const u16 t = p->v.u16[2051];
	chungus *chng = worldGetChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){return;}
	chunk *chnk = &chng->chunks[(x>>4)&0xF][(y>>4)&0xF][(z>>4)&0xF];
	void *dest;
	switch(t){
	case chunkOverlayBlock:
	default:
		if(chnk->block == NULL){chnk->block = chunkOverlayAllocate();}
		dest = &chnk->block->data[0][0][0];
		if(chunkOverlayCopyAndCompare(dest, p->v.u8)){chunkDirtyRegion(x, y, z, CHUNK_FLAG_DIRTY);}
		return;
	case chunkOverlayFluid:
		if(chnk->fluid == NULL){chnk->fluid = chunkOverlayAllocate();}
		dest = &chnk->fluid->data[0][0][0];
		if(chunkOverlayCopyAndCompare(dest, p->v.u8)){chunkDirtyRegion(x, y, z, CHUNK_FLAG_FLUID_DIRTY);}
		break;
	case chunkOverlayFire:
		if(chnk->flame == NULL){chnk->flame = chunkOverlayAllocate();}
		dest = &chnk->flame->data[0][0][0];
		break;
	}
	memcpy(dest,p->v.u8,sizeof(chnk->block->data));
}

void chunkDirtyAll(){
	for(uint i = 0; i < chungusCount; i++){
		chungus *cng = &chungusList[i];
		for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
		for(int z=0;z<16;z++){
			cng->chunks[x][y][z].flags |= CHUNK_FLAG_DIRTY;
		}
		}
		}
	}
}
