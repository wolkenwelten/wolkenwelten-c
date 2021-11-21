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
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gui/gui.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
#include "../sdl/sdl.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunkvertbuf.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

vertexTiny blockMeshBuffer[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6 / 2];
u16 blockMeshSideEnd[sideMAX];
int chunkFreeCount = 0;
int chunkCount     = 0;
int chunksGeneratedThisFrame = 0;
chunk *chunkFirstFree = NULL;

#define CHUNK_COUNT (1<<17)
#define MIN_CHUNKS_GENERATED_PER_FRAME (16)
#define FADE_IN_FRAMES 48

chunk *chunkList;

void chunkInit(){
	chunkvertbufInit();
	if(chunkList == NULL){
		chunkList = malloc(sizeof(chunk) * CHUNK_COUNT);
	}
	chunkFreeCount = 0;
	chunkCount     = 0;
	chunkFirstFree = 0;
	chunksGeneratedThisFrame = 0;
}

uint    chunkGetFree()               { return chunkFreeCount;           }
uint    chunkGetActive()             { return chunkCount;               }
uint    chunkGetGeneratedThisFrame() { return chunksGeneratedThisFrame; }
void    chunkResetCounter()          { chunksGeneratedThisFrame = 0;    }

#define POS_MASK (CHUNK_SIZE-1)

chunk *chunkNew(u16 x,u16 y,u16 z){
	chunk *c = NULL;
	if(chunkFirstFree == NULL){
		if(chunkCount+1 >= CHUNK_COUNT){
			if(!chnkChngOverflow){
				fprintf(stderr,"client chunkList Overflow!\n");
				chnkChngOverflow=true;
			}
			return NULL;
		}
		c = &chunkList[chunkCount++];
	}else{
		c = chunkFirstFree;
		if((c < &chunkList[0]) || (c > &chunkList[CHUNK_COUNT])){
			fprintf(stderr,"%p thats not a valid pointer... cfp=%i\n",c,chunkFreeCount);
			return NULL;
		}
		chunkFirstFree = c->nextFree;
		//fprintf(stderr,"--cfp=%i\n",chunkFreeCount);
		chunkFreeCount--;
	}
	c->x         = x & (~POS_MASK);
	c->y         = y & (~POS_MASK);
	c->z         = z & (~POS_MASK);
	c->flags     = CHUNK_FLAG_DIRTY;
	c->vertbuf   = NULL;
	memset(c->data,0,sizeof(c->data));
	return c;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	if((c < &chunkList[0]) || (c > &chunkList[CHUNK_COUNT])){
		fprintf(stderr,"WTF am I freing\n");
		return;
	}
	chunkvertbufFree(c);
	chunkFreeCount++;
	c->nextFree = chunkFirstFree;
	chunkFirstFree = c;
}

static inline void chunkFinish(chunk *c){
	if(!c->vertbuf){
		c->fadeIn = FADE_IN_FRAMES;
	}
	c->flags &= ~CHUNK_FLAG_DIRTY;
	u16 sideCounts[sideMAX];
	for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
		const uint cFirst = sideIndex == 0 ? 0 : blockMeshSideEnd[sideIndex-1];
		sideCounts[sideIndex] = blockMeshSideEnd[sideIndex] - cFirst;
	}
	chunkvertbufUpdate(c, blockMeshBuffer, sideCounts);
}

#define mkVert(x,y,z,w,h,bt,side) (vertexTiny){x,y,z,w,h,bt,side}

static void chunkAddFront(blockId b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideFront];
	vertexTiny *vt = &blockMeshBuffer[blockMeshSideEnd[sideFront]];
	blockMeshSideEnd[sideFront] += 6;
	*vt++ = mkVert(x  ,y  ,z+d,0,h,bt,sideFront);
	*vt++ = mkVert(x+w,y  ,z+d,w,h,bt,sideFront);
	*vt++ = mkVert(x+w,y+h,z+d,w,0,bt,sideFront);
	*vt++ = mkVert(x+w,y+h,z+d,w,0,bt,sideFront);
	*vt++ = mkVert(x  ,y+h,z+d,0,0,bt,sideFront);
	*vt++ = mkVert(x  ,y  ,z+d,0,h,bt,sideFront);
}
static void chunkAddBack(blockId b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)d;
	const u8 bt = blocks[b].tex[sideBack];
	vertexTiny *vt = &blockMeshBuffer[blockMeshSideEnd[sideBack]];
	blockMeshSideEnd[sideBack] += 6;
	*vt++ = mkVert(x  ,y  ,z  ,0,h,bt,sideBack);
	*vt++ = mkVert(x  ,y+h,z  ,0,0,bt,sideBack);
	*vt++ = mkVert(x+w,y+h,z  ,w,0,bt,sideBack);
	*vt++ = mkVert(x+w,y+h,z  ,w,0,bt,sideBack);
	*vt++ = mkVert(x+w,y  ,z  ,w,h,bt,sideBack);
	*vt++ = mkVert(x  ,y  ,z  ,0,h,bt,sideBack);
}
static void chunkAddTop(blockId b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideTop];
	vertexTiny *vt = &blockMeshBuffer[blockMeshSideEnd[sideTop]];
	blockMeshSideEnd[sideTop] += 6;
	*vt++ = mkVert(x  ,y+h,z  ,0,0,bt,sideTop);
	*vt++ = mkVert(x  ,y+h,z+d,0,d,bt,sideTop);
	*vt++ = mkVert(x+w,y+h,z+d,w,d,bt,sideTop);
	*vt++ = mkVert(x+w,y+h,z+d,w,d,bt,sideTop);
	*vt++ = mkVert(x+w,y+h,z  ,w,0,bt,sideTop);
	*vt++ = mkVert(x  ,y+h,z  ,0,0,bt,sideTop);
}
static void chunkAddBottom(blockId b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)h;
	const u8 bt = blocks[b].tex[sideBottom];
	vertexTiny *vt = &blockMeshBuffer[blockMeshSideEnd[sideBottom]];
	blockMeshSideEnd[sideBottom] += 6;
	*vt++ = mkVert(x  ,y  ,z  ,0,0,bt,sideBottom);
	*vt++ = mkVert(x+w,y  ,z  ,w,0,bt,sideBottom);
	*vt++ = mkVert(x+w,y  ,z+d,w,d,bt,sideBottom);
	*vt++ = mkVert(x+w,y  ,z+d,w,d,bt,sideBottom);
	*vt++ = mkVert(x  ,y  ,z+d,0,d,bt,sideBottom);
	*vt++ = mkVert(x  ,y  ,z  ,0,0,bt,sideBottom);
}
static void chunkAddLeft(blockId b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)w;
	const u8 bt = blocks[b].tex[sideLeft];
	vertexTiny *vt = &blockMeshBuffer[blockMeshSideEnd[sideLeft]];
	blockMeshSideEnd[sideLeft] += 6;
	*vt++ = mkVert(x  ,y  ,z  ,0,h,bt,sideLeft);
	*vt++ = mkVert(x  ,y  ,z+d,d,h,bt,sideLeft);
	*vt++ = mkVert(x  ,y+h,z+d,d,0,bt,sideLeft);
	*vt++ = mkVert(x  ,y+h,z+d,d,0,bt,sideLeft);
	*vt++ = mkVert(x  ,y+h,z  ,0,0,bt,sideLeft);
	*vt++ = mkVert(x  ,y  ,z  ,0,h,bt,sideLeft);
}
static void chunkAddRight(blockId b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideRight];
	vertexTiny *vt = &blockMeshBuffer[blockMeshSideEnd[sideRight]];
	blockMeshSideEnd[sideRight] += 6;
	*vt++ = mkVert(x+w,y  ,z  ,0,h,bt,sideRight);
	*vt++ = mkVert(x+w,y+h,z  ,0,0,bt,sideRight);
	*vt++ = mkVert(x+w,y+h,z+d,d,0,bt,sideRight);
	*vt++ = mkVert(x+w,y+h,z+d,d,0,bt,sideRight);
	*vt++ = mkVert(x+w,y  ,z+d,d,h,bt,sideRight);
	*vt++ = mkVert(x+w,y  ,z  ,0,h,bt,sideRight);
}

static void chunkOptimizePlane(u32 plane[CHUNK_SIZE][CHUNK_SIZE]){
	for(int y=CHUNK_SIZE-1;y>=0;y--){
	for(int x=CHUNK_SIZE-1;x>=0;x--){
		if((x < CHUNK_SIZE-1) && (plane[x][y]) && ((plane[x][y] & 0xFF00FF) == (plane[x+1][y] & 0xFF00FF))){
			plane[x  ][y] += plane[x+1][y] & 0xFF00;
			plane[x+1][y]  = 0;
		}
		if((y < CHUNK_SIZE-1) && (plane[x][y]) && ((plane[x][y] & 0x00FFFF) == (plane[x][y+1] & 0x00FFFF))){
			plane[x][y  ] += plane[x][y+1]&0xFF0000;
			plane[x][y+1]  = 0;
		}
	}
	}
}

static inline sideMask chunkGetSides(u16 x,u16 y,u16 z,blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2]){
	sideMask sides = 0;

	if(b[x][y][z+1] == 0){ sides |= sideMaskFront; }
	if(b[x][y][z-1] == 0){ sides |= sideMaskBack; }
	if(b[x][y+1][z] == 0){ sides |= sideMaskTop; }
	if(b[x][y-1][z] == 0){ sides |= sideMaskBottom; }
	if(b[x+1][y][z] == 0){ sides |= sideMaskLeft; }
	if(b[x-1][y][z] == 0){ sides |= sideMaskRight; }

	return sides;
}

static void chunkPopulateBlockData(blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff){
	if(c == NULL){return;}
	for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
	for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
	for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
		b[x][y][z] = c->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}

static void chunkGenMesh(chunk *c) {
	PROFILE_START();

	if((chunksGeneratedThisFrame >= MIN_CHUNKS_GENERATED_PER_FRAME) && (getTicks() > frameRelaxedDeadline)){return;}
	static blockId  blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static sideMask sideCache[CHUNK_SIZE  ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	static u32          plane[CHUNK_SIZE  ][CHUNK_SIZE  ];
	++chunksGeneratedThisFrame;
	memset(blockData,   0,sizeof(blockData));
	chunkPopulateBlockData(blockData,c,1,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x-CHUNK_SIZE,c->y,c->z),1-CHUNK_SIZE,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x+CHUNK_SIZE,c->y,c->z),1+CHUNK_SIZE,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y-CHUNK_SIZE,c->z),1,1-CHUNK_SIZE,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y+CHUNK_SIZE,c->z),1,1+CHUNK_SIZE,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z-CHUNK_SIZE),1,1,1-CHUNK_SIZE);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z+CHUNK_SIZE),1,1,1+CHUNK_SIZE);

	for(int x=CHUNK_SIZE-1;x>=0;--x){
	for(int y=CHUNK_SIZE-1;y>=0;--y){
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		sideCache[x][y][z] = c->data[x][y][z] == 0 ? 0 : chunkGetSides(x+1,y+1,z+1,blockData);
	}
	}
	}

	blockMeshSideEnd[sideFront] = 0;
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] &sideMaskFront){
				found = true;
				plane[y][x] = b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cw = ((plane[y][x] >> 16) & 0xFF);
				const int ch = ((plane[y][x] >>  8) & 0xFF);
				const blockId b = plane[y][x] & 0xFF;
				chunkAddFront(b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	blockMeshSideEnd[sideBack] = blockMeshSideEnd[sideFront];
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskBack){
				found = true;
				plane[y][x] = b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cw = ((plane[y][x] >> 16) & 0xFF);
				const int ch = ((plane[y][x] >>  8) & 0xFF);
				const blockId b = plane[y][x] & 0xFF;
				chunkAddBack(b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	blockMeshSideEnd[sideTop] = blockMeshSideEnd[sideBack];
	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskTop){
				found = true;
				plane[z][x] = b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cw = ((plane[z][x] >> 16) & 0xFF);
				const int cd = ((plane[z][x] >>  8) & 0xFF);
				const blockId b = plane[z][x] & 0xFF;
				chunkAddTop(b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	blockMeshSideEnd[sideBottom] = blockMeshSideEnd[sideTop];
	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskBottom){
				found = true;
				plane[z][x] = b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cw = ((plane[z][x] >> 16) & 0xFF);
				const int cd = ((plane[z][x] >>  8) & 0xFF);
				const blockId b = plane[z][x] & 0xFF;
				chunkAddBottom(b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	blockMeshSideEnd[sideLeft] = blockMeshSideEnd[sideBottom];
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = c->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskRight){
				found = true;
				plane[y][z] = b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int ch = ((plane[y][z] >>  8) & 0xFF);
				const int cd = ((plane[y][z] >> 16) & 0xFF);
				const blockId b = plane[y][z] & 0xFF;
				chunkAddLeft(b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	blockMeshSideEnd[sideRight] = blockMeshSideEnd[sideLeft];
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = c->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskLeft){
				found = true;
				plane[y][z] = b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int ch = ((plane[y][z] >>  8) & 0xFF);
				const int cd = ((plane[y][z] >> 16) & 0xFF);
				const blockId b = plane[y][z] & 0xFF;
				chunkAddRight(b,x,y,z,cw,ch,cd);
			}
			}
		}
	}
	chunkFinish(c);

	PROFILE_STOP();
}

#define EDGE (CHUNK_SIZE-1)

void chunkBox(chunk *c, u16 x,u16 y,u16 z,u16 gx,u16 gy,u16 gz,blockId block){
	for(int cx=x;cx<gx;cx++){
	for(int cy=y;cy<gy;cy++){
	for(int cz=z;cz<gz;cz++){
		c->data[cx][cy][cz] = block;
	}
	}
	}
	c->flags |= CHUNK_FLAG_DIRTY;
	if(( x&EDGE) ==  0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if(( y&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if(( z&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((gx&EDGE) == EDGE){worldSetChunkUpdated(x+1,y  ,z  );}
	if((gy&EDGE) == EDGE){worldSetChunkUpdated(x  ,y+1,z  );}
	if((gz&EDGE) == EDGE){worldSetChunkUpdated(x  ,y  ,z+1);}
}

void chunkSetB(chunk *c,u16 x,u16 y,u16 z,blockId block){
	c->data[x&POS_MASK][y&POS_MASK][z&POS_MASK] = block;
	c->flags |= CHUNK_FLAG_DIRTY;
	if((x&EDGE) ==  0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if((x&EDGE) == EDGE){worldSetChunkUpdated(x+1,y  ,z  );}
	if((y&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if((y&EDGE) == EDGE){worldSetChunkUpdated(x  ,y+1,z  );}
	if((z&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((z&EDGE) == EDGE){worldSetChunkUpdated(x  ,y  ,z+1);}
}

static void chunkDraw(chunk *c, float d, sideMask mask){
	if(c == NULL){return;}
	if(c->flags & CHUNK_FLAG_DIRTY){ chunkGenMesh(c); }

	// Since chunk mesh generation & upload is rate limited, we might not have a vertbuf yet.
	// Trying to draw with an empty vertbuf would dereference null, avoid that.
	if(c->vertbuf == NULL){return;}

	float fIn = 1.f;
	if(c->fadeIn > 0){
		fIn = 1.f - ((--c->fadeIn) / (float)FADE_IN_FRAMES);
	}

	if(d > (fadeoutStartDistance)){
		shaderAlpha(sBlockMesh,(1.f-((d-(fadeoutStartDistance))/fadeoutDistance)) * fIn);
	}else{
		shaderAlpha(sBlockMesh,1.f * fIn);
	}

	chunkvertbufDrawOne(c,mask);
}

void chunkDrawQueue(queueEntry *queue, int queueLen, const vec sideTints[sideMAX]){
	shaderSideTints(sBlockMesh,sideTints);
	for(int i=0;i<queueLen;i++){
		chunkDraw(queue[i].chnk,queue[i].distance,queue[i].mask);
	}
}

void chunkRecvUpdate(const packet *p){
	u16 x = p->v.u16[2048];
	u16 y = p->v.u16[2049];
	u16 z = p->v.u16[2050];
	chungus *chng =  worldGetChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){return;}
	chunk *chnk = chungusGetChunkOrNew(chng,x,y,z);
	if(chnk == NULL){return;}
	memcpy(chnk->data,p->v.u8,sizeof(chnk->data));
	chnk->flags |= CHUNK_FLAG_DIRTY;
}
