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

vertexTiny blockMeshBuffer[sideMAX][CHUNK_SIZE * CHUNK_SIZE * 2 * 3];
int chunkFreeCount = 0;
int chunkCount     = 0;
int chunksGeneratedThisFrame = 0;
chunk *chunkFirstFree = NULL;

#ifdef __EMSCRIPTEN__
	#define MAX_CHUNKS_GEN_PER_FRAME 16
#elif defined(__aarch64__) || defined(__ARM_ARCH_7A__)
	#define MAX_CHUNKS_GEN_PER_FRAME 8
#else
	#define MAX_CHUNKS_GEN_PER_FRAME 48
#endif

#define CHUNK_COUNT (1<<17)

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
	}else{
		glBindVertexArray(c->vao);
	}
	if(!c->vbo) { glGenBuffers(1,&c->vbo); }
	glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
	const u16 quadCount =
		c->sideQuads[sideFront].count +
		c->sideQuads[sideBack].count +
		c->sideQuads[sideTop].count +
		c->sideQuads[sideBottom].count +
		c->sideQuads[sideLeft].count +
		c->sideQuads[sideRight].count;
	if (c->vboSize < quadCount) {
		glBufferData(GL_ARRAY_BUFFER, quadCount*(6*sizeof(vertexTiny)), NULL, GL_STATIC_DRAW);
		c->vboSize = quadCount;
	}
	u16 offset = 0;
	for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
		glBufferSubData(GL_ARRAY_BUFFER, offset*(6*sizeof(vertexTiny)), c->sideQuads[sideIndex].count*(6*sizeof(vertexTiny)), blockMeshBuffer[sideIndex]);
		c->sideQuads[sideIndex].offset = offset;
		offset += c->sideQuads[sideIndex].count;
	}
	glVertexAttribPointer(0, 3, GL_BYTE,          GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, x));
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, u));
	glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, f));
	c->flags &= ~CHUNK_FLAG_DIRTY;
}

static inline void chunkAddFront(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideFront];
	vertexTiny *vt = &blockMeshBuffer[sideFront][c->sideQuads[sideFront].count++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,h,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,h,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,0,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z+d,0,0,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,h,bt,2};
}
static inline void chunkAddBack(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)d;
	const u8 bt = blocks[b].tex[sideBack];
	vertexTiny *vt = &blockMeshBuffer[sideBack][c->sideQuads[sideBack].count++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z  ,w,h,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
}
static inline void chunkAddTop(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideTop];
	vertexTiny *vt = &blockMeshBuffer[sideTop][c->sideQuads[sideTop].count++ * 6];
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,3};
	*vt++ = (vertexTiny){x  ,y+h,z+d,0,d,bt,3};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,d,bt,3};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,d,bt,3};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,3};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,3};
}
static inline void chunkAddBottom(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)h;
	const u8 bt = blocks[b].tex[sideBottom];
	vertexTiny *vt = &blockMeshBuffer[sideBottom][c->sideQuads[sideBottom].count++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,0,bt,0};
	*vt++ = (vertexTiny){x+w,y  ,z  ,w,0,bt,0};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,d,bt,0};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,d,bt,0};
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,d,bt,0};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,0,bt,0};
}
static inline void chunkAddRight(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[sideRight];
	vertexTiny *vt = &blockMeshBuffer[sideRight][c->sideQuads[sideRight].count++ * 6];
	*vt++ = (vertexTiny){x+w,y  ,z  ,0,h,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z  ,0,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z+d,d,h,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z  ,0,h,bt,2};
}
static inline void chunkAddLeft(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)w;
	const u8 bt = blocks[b].tex[sideLeft];
	vertexTiny *vt = &blockMeshBuffer[sideLeft][c->sideQuads[sideLeft].count++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z+d,d,h,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
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

	int counts[sideMAX];
	static u8 blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static u8 sideCache[CHUNK_SIZE  ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	static u32    plane[sideMAX     ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	if(++chunksGeneratedThisFrame > MAX_CHUNKS_GEN_PER_FRAME){return;}
	memset(c->sideQuads,0,sizeof(c->sideQuads));
	memset(plane,       0,sizeof(plane));
	memset(counts,      0,sizeof(counts));
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
		if(c->data[x][y][z] == 0){
			sideCache[x][y][z] = 0;
		}else{
			sideCache[x][y][z] = chunkGetSides(x+1,y+1,z+1,blockData);
		}
	}
	}
	}

	// Front / Back
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		memset(plane[sideFront],0,sizeof(plane[sideFront]));
		memset(plane[sideBack],0,sizeof(plane[sideBack]));
		counts[sideFront] = counts[sideBack] = 0;
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const u8 b = c->data[x][y][z];
			if(b == 0){continue;}
			const u8 sides = sideCache[x][y][z];
			if(sides&sideMaskFront){
				counts[sideFront]++;
				plane[sideFront][y][x] = b | 0x010100;
			}
			if(sides&sideMaskBack){
				counts[sideBack]++;
				plane[sideBack][y][x] = b | 0x010100;
			}
		}
		}
		chunkOptimizePlane(plane[sideFront]);
		chunkOptimizePlane(plane[sideBack]);
		if(counts[sideFront] || counts[sideBack]){
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(plane[sideFront][y][x]){
					const int cw = ((plane[sideFront][y][x] >> 16) & 0xFF);
					const int ch = ((plane[sideFront][y][x] >>  8) & 0xFF);
					const u8 b = plane[sideFront][y][x] & 0xFF;
					chunkAddFront(c,b,x,y,z,cw,ch,cd);
				}
				if(plane[sideBack][y][x]){
					const int cw = ((plane[sideBack][y][x] >> 16) & 0xFF);
					const int ch = ((plane[sideBack][y][x] >>  8) & 0xFF);
					const u8 b = plane[sideBack][y][x] & 0xFF;
					chunkAddBack(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}

	// Top / Bottom
	for(int y=CHUNK_SIZE-1;y>=0;--y){
		memset(plane[sideTop],0,sizeof(plane[sideTop]));
		memset(plane[sideBottom],0,sizeof(plane[sideBottom]));
		counts[sideTop] = counts[sideBottom] = 0;
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const u8 b = c->data[x][y][z];
			if(b == 0){continue;}
			const u8 sides = sideCache[x][y][z];
			if(sides&sideMaskTop){
				counts[sideTop]++;
				plane[sideTop][z][x] = b | 0x010100;
			}
			if(sides&sideMaskBottom){
				counts[sideBottom]++;
				plane[sideBottom][z][x] = b | 0x010100;
			}
		}
		}
		chunkOptimizePlane(plane[sideTop]);
		chunkOptimizePlane(plane[sideBottom]);
		if(counts[sideTop] || counts[sideBottom]){
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(plane[sideTop][z][x]){
					const int cw = ((plane[sideTop][z][x] >> 16) & 0xFF);
					const int cd = ((plane[sideTop][z][x] >>  8) & 0xFF);
					const u8 b = plane[sideTop][z][x] & 0xFF;
					chunkAddTop(c,b,x,y,z,cw,ch,cd);
				}
				if(plane[sideBottom][z][x]){
					const int cw = ((plane[sideBottom][z][x] >> 16) & 0xFF);
					const int cd = ((plane[sideBottom][z][x] >>  8) & 0xFF);
					const u8 b = plane[sideBottom][z][x] & 0xFF;
					chunkAddBottom(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}

	// Right / Left
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		memset(plane[sideLeft],0,sizeof(plane[sideLeft]));
		memset(plane[sideRight],0,sizeof(plane[sideRight]));
		counts[sideLeft] = counts[sideRight] = 0;
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const u8 b = c->data[x][y][z];
			if(b == 0){continue;}
			const u8 sides = sideCache[x][y][z];
			if(sides&sideMaskLeft){
				counts[sideLeft]++;
				plane[sideLeft][y][z] = b | 0x010100;
			}
			if(sides&sideMaskRight){
				counts[sideRight]++;
				plane[sideRight][y][z] = b | 0x010100;
			}
		}
		}
		chunkOptimizePlane(plane[sideLeft]);
		chunkOptimizePlane(plane[sideRight]);
		if(counts[sideLeft] || counts[sideRight]){
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(plane[sideLeft][y][z]){
					const int ch = ((plane[sideLeft][y][z] >>  8) & 0xFF);
					const int cd = ((plane[sideLeft][y][z] >> 16) & 0xFF);
					const u8 b = plane[sideLeft][y][z] & 0xFF;
					chunkAddRight(c,b,x,y,z,cw,ch,cd);
				}
				if(plane[sideRight][y][z]){
					const int ch = ((plane[sideRight][y][z] >>  8) & 0xFF);
					const int cd = ((plane[sideRight][y][z] >> 16) & 0xFF);
					const u8 b = plane[sideRight][y][z] & 0xFF;
					chunkAddLeft(c,b,x,y,z,cw,ch,cd);
				}
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
	if(d > (fadeoutStartDistance)){
		shaderAlpha(sBlockMesh,(1.f-((d-(fadeoutStartDistance))/fadeoutDistance)));
	}else{
		shaderAlpha(sBlockMesh,1.f);
	}
	shaderTransform(sBlockMesh,c->x,c->y,c->z);

	glBindVertexArray(c->vao);
	if(mask == sideMaskALL || !glIsMultiDrawAvailable){
		const u16 quadCount =
			c->sideQuads[sideFront].count +
			c->sideQuads[sideBack].count +
			c->sideQuads[sideTop].count +
			c->sideQuads[sideBottom].count +
			c->sideQuads[sideLeft].count +
			c->sideQuads[sideRight].count;
		glDrawArrays(GL_TRIANGLES,0,quadCount*6);
		vboTrisCount += quadCount*2;
		drawCallCount++;
	}else{
		GLint first[sideMAX];
		GLsizei count[sideMAX];
		u16 index = 0, quadCount = 0;
		for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
			if(mask & (1 << sideIndex)){
				first[index] = c->sideQuads[sideIndex].offset*6;
				count[index] = c->sideQuads[sideIndex].count*6;
				quadCount += c->sideQuads[sideIndex].count;
				index++;
			}
		}
		glMultiDrawArrays(GL_TRIANGLES,first,count,__builtin_popcount(mask));
		vboTrisCount += quadCount*2;
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
