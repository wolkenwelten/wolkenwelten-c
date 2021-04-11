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
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct vertexTiny {
	u8 x,y,z,u,v,w,f;
} vertexTiny;
#pragma pack(pop)

vertexTiny blockMeshBuffer[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6];
int chunkFreeCount = 0;
int chunkCount     = 0;
int chunksGeneratedThisFrame = 0;
chunk *chunkFirstFree = NULL;

#define CHUNK_COUNT (1<<17)
#define MIN_CHUNKS_GENERATED_PER_FRAME (4)
#define FADE_IN_FRAMES 48

chunk *chunkList;

void chunkInit(){
	chunkList = malloc(sizeof(chunk) * CHUNK_COUNT);
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
	c->flags     = 0;
	c->vbo       = 0;
	c->vboSize   = 0;
	c->vao       = 0;
	memset(c->data,0,sizeof(c->data));
	return c;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	if((c < &chunkList[0]) || (c > &chunkList[CHUNK_COUNT])){
		fprintf(stderr,"WTF am I freing\n");
		return;
	}
	chunkFreeCount++;
	if(c->vbo){glDeleteBuffers(1,&c->vbo); c->vbo = 0;}
	if(c->vao){glDeleteVertexArrays(1,&c->vao); c->vao = 0;}
	c->nextFree = chunkFirstFree;
	chunkFirstFree = c;
}

static inline void chunkFinish(chunk *c){
	if(!c->vao) {
		glGenVertexArrays(1, &c->vao);
		glBindVertexArray(c->vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		c->fadeIn = FADE_IN_FRAMES;
	}else{
		glBindVertexArray(c->vao);
	}
	if(!c->vbo) { glGenBuffers(1,&c->vbo); }
	glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
	if(gfxUseSubData && (c->vboSize >= c->sideEnd[sideMAX-1])){
		glBufferSubData(GL_ARRAY_BUFFER, 0, c->sideEnd[sideMAX-1] * (6 * sizeof(vertexTiny)), blockMeshBuffer);
	}else{
		glBufferData(GL_ARRAY_BUFFER, c->sideEnd[sideMAX-1] * (6 * sizeof(vertexTiny)), blockMeshBuffer, GL_STATIC_DRAW);
		c->vboSize = c->sideEnd[sideMAX-1];
	}
	glVertexAttribPointer(0, 3, GL_BYTE,          GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, x));
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, u));
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, f));
	c->flags &= ~CHUNK_FLAG_DIRTY;
}

static void chunkAddFront(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideFront];
	vertexTiny *vt = &blockMeshBuffer[c->sideEnd[sideFront]++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,h,bt,sideFront};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,h,bt,sideFront};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,0,bt,sideFront};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,0,bt,sideFront};
	*vt++ = (vertexTiny){x  ,y+h,z+d,0,0,bt,sideFront};
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,h,bt,sideFront};
}
static void chunkAddBack(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)d;
	const u8 bt = blocks[b].tex[sideBack];
	vertexTiny *vt = &blockMeshBuffer[c->sideEnd[sideBack]++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,sideBack};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,sideBack};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,sideBack};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,sideBack};
	*vt++ = (vertexTiny){x+w,y  ,z  ,w,h,bt,sideBack};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,sideBack};
}
static void chunkAddTop(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideTop];
	vertexTiny *vt = &blockMeshBuffer[c->sideEnd[sideTop]++ * 6];
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,sideTop};
	*vt++ = (vertexTiny){x  ,y+h,z+d,0,d,bt,sideTop};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,d,bt,sideTop};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,d,bt,sideTop};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,sideTop};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,sideTop};
}
static void chunkAddBottom(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)h;
	const u8 bt = blocks[b].tex[sideBottom];
	vertexTiny *vt = &blockMeshBuffer[c->sideEnd[sideBottom]++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,0,bt,sideBottom};
	*vt++ = (vertexTiny){x+w,y  ,z  ,w,0,bt,sideBottom};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,d,bt,sideBottom};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,d,bt,sideBottom};
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,d,bt,sideBottom};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,0,bt,sideBottom};
}
static void chunkAddLeft(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)w;
	const u8 bt = blocks[b].tex[sideLeft];
	vertexTiny *vt = &blockMeshBuffer[c->sideEnd[sideLeft]++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,sideLeft};
	*vt++ = (vertexTiny){x  ,y  ,z+d,d,h,bt,sideLeft};
	*vt++ = (vertexTiny){x  ,y+h,z+d,d,0,bt,sideLeft};
	*vt++ = (vertexTiny){x  ,y+h,z+d,d,0,bt,sideLeft};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,sideLeft};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,sideLeft};
}
static void chunkAddRight(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideRight];
	vertexTiny *vt = &blockMeshBuffer[c->sideEnd[sideRight]++ * 6];
	*vt++ = (vertexTiny){x+w,y  ,z  ,0,h,bt,sideRight};
	*vt++ = (vertexTiny){x+w,y+h,z  ,0,0,bt,sideRight};
	*vt++ = (vertexTiny){x+w,y+h,z+d,d,0,bt,sideRight};
	*vt++ = (vertexTiny){x+w,y+h,z+d,d,0,bt,sideRight};
	*vt++ = (vertexTiny){x+w,y  ,z+d,d,h,bt,sideRight};
	*vt++ = (vertexTiny){x+w,y  ,z  ,0,h,bt,sideRight};
}

static void chunkOptimizePlane(u32 plane[CHUNK_SIZE][CHUNK_SIZE]){
	for(int y=CHUNK_SIZE-1;y>=0;y--){
	for(int x=CHUNK_SIZE-2;x>=0;x--){
		if(!plane[x][y])                                       {continue;}
		if((plane[x][y] & 0xFF00FF) != (plane[x+1][y] & 0xF0FF)) {continue;}
		plane[x  ][y] += plane[x+1][y] & 0xFF00;
		plane[x+1][y]  = 0;
	}
	}
	for(int x=CHUNK_SIZE-1;x>=0;x--){
	for(int y=CHUNK_SIZE-2;y>=0;y--){
		if(!plane[x][y])                                   {continue;}
		if((plane[x][y]&0xFFFF) != (plane[x][y+1]&0xFFFF)) {continue;}
		plane[x][y  ] += plane[x][y+1]&0xFF0000;
		plane[x][y+1]  = 0;
	}
	}
}

static inline u8 chunkGetSides(u16 x,u16 y,u16 z,u8 b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2]){
	u8 sides = 0;

	if(b[x][y][z+1] == 0){ sides |= sideMaskFront; }
	if(b[x][y][z-1] == 0){ sides |= sideMaskBack; }
	if(b[x][y+1][z] == 0){ sides |= sideMaskTop; }
	if(b[x][y-1][z] == 0){ sides |= sideMaskBottom; }
	if(b[x+1][y][z] == 0){ sides |= sideMaskLeft; }
	if(b[x-1][y][z] == 0){ sides |= sideMaskRight; }

	return sides;
}

static void chunkPopulateBlockData(u8 b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff){
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

	static u8 blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static u8 sideCache[CHUNK_SIZE  ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	static u32    plane[CHUNK_SIZE  ][CHUNK_SIZE  ];
	if((chunksGeneratedThisFrame >= MIN_CHUNKS_GENERATED_PER_FRAME) && (getTicks() > frameRelaxedDeadline)){return;}
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

	c->sideEnd[sideFront] = 0;
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const u8 b = c->data[x][y][z];
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
				const u8 b = plane[y][x] & 0xFF;
				chunkAddFront(c,b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	c->sideEnd[sideBack] = c->sideEnd[sideFront];
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const u8 b = c->data[x][y][z];
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
				const u8 b = plane[y][x] & 0xFF;
				chunkAddBack(c,b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	c->sideEnd[sideTop] = c->sideEnd[sideBack];
	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const u8 b = c->data[x][y][z];
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
				const u8 b = plane[z][x] & 0xFF;
				chunkAddTop(c,b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	c->sideEnd[sideBottom] = c->sideEnd[sideTop];
	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const u8 b = c->data[x][y][z];
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
				const u8 b = plane[z][x] & 0xFF;
				chunkAddBottom(c,b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	c->sideEnd[sideLeft] = c->sideEnd[sideBottom];
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const u8 b = c->data[x][y][z];
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
				const u8 b = plane[y][z] & 0xFF;
				chunkAddLeft(c,b,x,y,z,cw,ch,cd);
			}
			}
		}
	}

	c->sideEnd[sideRight] = c->sideEnd[sideLeft];
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const u8 b = c->data[x][y][z];
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
				const u8 b = plane[y][z] & 0xFF;
				chunkAddRight(c,b,x,y,z,cw,ch,cd);
			}
			}
		}
	}
	chunkFinish(c);

	PROFILE_STOP();
}

#define EDGE (CHUNK_SIZE-1)

void chunkBox(chunk *c, u16 x,u16 y,u16 z,u16 gx,u16 gy,u16 gz,u8 block){
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

void chunkSetB(chunk *c,u16 x,u16 y,u16 z,u8 block){
	c->data[x&POS_MASK][y&POS_MASK][z&POS_MASK] = block;
	c->flags |= CHUNK_FLAG_DIRTY;
	if((x&EDGE) ==  0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if((x&EDGE) == EDGE){worldSetChunkUpdated(x+1,y  ,z  );}
	if((y&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if((y&EDGE) == EDGE){worldSetChunkUpdated(x  ,y+1,z  );}
	if((z&EDGE) ==  0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((z&EDGE) == EDGE){worldSetChunkUpdated(x  ,y  ,z+1);}
}

void chunkDraw(chunk *c, float d, sideMask mask){
	if(c == NULL){return;}
	if(c->flags & CHUNK_FLAG_DIRTY){ chunkGenMesh(c); }
	if(!c->vao){ return; }

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

	glBindVertexArray(c->vao);
	if(mask == sideMaskALL || !glIsMultiDrawAvailable){
		glDrawArrays(GL_TRIANGLES,0,c->sideEnd[sideMAX-1]*6);
		vboTrisCount += c->sideEnd[sideMAX-1] * 2;
		drawCallCount++;
	}else{
		GLint first[sideMAX];
		GLsizei count[sideMAX];
		uint index = 0;
		bool lastSide = false;
		for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
			if(mask & (1 << sideIndex)){
				const uint cFirst = sideIndex == 0 ? 0 : c->sideEnd[sideIndex-1];
				const uint cCount = c->sideEnd[sideIndex] - cFirst;
				if(cCount == 0){continue;}
				if(lastSide){count[index-1] += cCount * 6;}
				first[index]   = cFirst * 6;
				count[index++] = cCount * 6;
				vboTrisCount  += cCount * 2;
				lastSide = true;
			}else{
				lastSide = false;
			}
		}
		glMultiDrawArrays(GL_TRIANGLES,first,count,index);
		drawCallCount++;
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
