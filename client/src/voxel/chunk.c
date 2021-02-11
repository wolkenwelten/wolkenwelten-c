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

vertexTiny blockMeshBuffer[1<<16];
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
	c->x         = x & (~0xF);
	c->y         = y & (~0xF);
	c->z         = z & (~0xF);
	c->vbo       = 0;
	c->vboSize   = 0;
	c->vao       = 0;
	c->dataCount = 0xFFFF;
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
	if(gfxUseSubData && (c->vboSize >= c->dataCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, c->dataCount*(6*sizeof(vertexTiny)), blockMeshBuffer);
	}else{
		glBufferData(GL_ARRAY_BUFFER, c->dataCount*(6*sizeof(vertexTiny)), blockMeshBuffer, GL_DYNAMIC_DRAW);
		c->vboSize = c->dataCount;
	}
	glVertexAttribPointer(0, 3, GL_BYTE,          GL_FALSE, sizeof(vertexTiny), (void *)(((char *)&blockMeshBuffer[0].x) - ((char *)blockMeshBuffer)));
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)(((char *)&blockMeshBuffer[0].u) - ((char *)blockMeshBuffer)));
	glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)(((char *)&blockMeshBuffer[0].f) - ((char *)blockMeshBuffer)));
}

static inline void chunkAddFront(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[0];
	vertexTiny *vt = &blockMeshBuffer[c->dataCount++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,h,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,h,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,0,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z+d,0,0,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,h,bt,2};
}
static inline void chunkAddBack(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)d;
	const u8 bt = blocks[b].tex[1];
	vertexTiny *vt = &blockMeshBuffer[c->dataCount++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z  ,w,h,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
}
static inline void chunkAddTop(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[2];
	vertexTiny *vt = &blockMeshBuffer[c->dataCount++ * 6];
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,3};
	*vt++ = (vertexTiny){x  ,y+h,z+d,0,d,bt,3};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,d,bt,3};
	*vt++ = (vertexTiny){x+w,y+h,z+d,w,d,bt,3};
	*vt++ = (vertexTiny){x+w,y+h,z  ,w,0,bt,3};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,3};
}
static inline void chunkAddBottom(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)h;
	const u8 bt = blocks[b].tex[3];
	vertexTiny *vt = &blockMeshBuffer[c->dataCount++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,0,bt,0};
	*vt++ = (vertexTiny){x+w,y  ,z  ,w,0,bt,0};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,d,bt,0};
	*vt++ = (vertexTiny){x+w,y  ,z+d,w,d,bt,0};
	*vt++ = (vertexTiny){x  ,y  ,z+d,0,d,bt,0};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,0,bt,0};
}
static inline void chunkAddRight(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[4];
	vertexTiny *vt = &blockMeshBuffer[c->dataCount++ * 6];
	*vt++ = (vertexTiny){x+w,y  ,z  ,0,h,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z  ,0,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x+w,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z+d,d,h,bt,2};
	*vt++ = (vertexTiny){x+w,y  ,z  ,0,h,bt,2};
}
static inline void chunkAddLeft(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)w;
	const u8 bt = blocks[b].tex[5];
	vertexTiny *vt = &blockMeshBuffer[c->dataCount++ * 6];
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z+d,d,h,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z+d,d,0,bt,2};
	*vt++ = (vertexTiny){x  ,y+h,z  ,0,0,bt,2};
	*vt++ = (vertexTiny){x  ,y  ,z  ,0,h,bt,2};
}

static void chunkOptimizePlane(u32 plane[CHUNK_SIZE][CHUNK_SIZE]){
	for(int y=15;y>=0;y--){
	for(int x=14;x>=0;x--){
		if(!plane[x][y])                                       {continue;}
		if((plane[x][y] & 0xFF00FF) != (plane[x+1][y] & 0xF0FF)) {continue;}
		plane[x  ][y] += plane[x+1][y] & 0xFF00;
		plane[x+1][y]  = 0;
	}
	}
	for(int x=15;x>=0;x--){
	for(int y=14;y>=0;y--){
		if(!plane[x][y])                                   {continue;}
		if((plane[x][y]&0xFFFF) != (plane[x][y+1]&0xFFFF)) {continue;}
		plane[x][y  ] += plane[x][y+1]&0xFF0000;
		plane[x][y+1]  = 0;
	}
	}
}

static inline u8 chunkGetSides(u16 x,u16 y,u16 z,u8 b[18][18][18]){
	u8 sides = 0;

	if(b[x][y][z+1] == 0){ sides |=  1;}
	if(b[x][y][z-1] == 0){ sides |=  2;}
	if(b[x][y+1][z] == 0){ sides |=  4;}
	if(b[x][y-1][z] == 0){ sides |=  8;}
	if(b[x+1][y][z] == 0){ sides |= 16;}
	if(b[x-1][y][z] == 0){ sides |= 32;}

	return sides;
}

static void chunkPopulateBlockData(u8 b[18][18][18], chunk *c, i16 xoff, i16 yoff, i16 zoff){
	if(c == NULL){return;}
	for(int x=MAX(0,xoff); x<MIN(18,xoff+16); x++){
	for(int y=MAX(0,yoff); y<MIN(18,yoff+16); y++){
	for(int z=MAX(0,zoff); z<MIN(18,zoff+16); z++){
		b[x][y][z] = c->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}

static void chunkGenMesh(chunk *c) {
	PROFILE_START();

	int counts[6];
	static u8 blockData[18][18][18];
	static u8 sideCache[16][16][16];
	static u32     plane[6][16][16];
	if(++chunksGeneratedThisFrame > MAX_CHUNKS_GEN_PER_FRAME){return;}
	c->dataCount = 0;
	memset(plane,    0,sizeof(plane));
	memset(counts,   0,sizeof(counts));
	memset(blockData,0,sizeof(blockData));
	chunkPopulateBlockData(blockData,c,1,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x-16,c->y,c->z),1-16,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x+16,c->y,c->z),1+16,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y-16,c->z),1,1-16,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y+16,c->z),1,1+16,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z-16),1,1,1-16);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z+16),1,1,1+16);

	for(int x=15;x>=0;--x){
	for(int y=15;y>=0;--y){
	for(int z=15;z>=0;--z){
		if(c->data[x][y][z] == 0){
			sideCache[x][y][z] = 0;
		}else{
			sideCache[x][y][z] = chunkGetSides(x+1,y+1,z+1,blockData);
		}
	}
	}
	}

	// Front / Back
	for(int z=15;z>=0;--z){
		memset(plane[0],0,sizeof(plane[0]));
		memset(plane[1],0,sizeof(plane[1]));
		counts[0] = counts[1] = 0;
		for(int y=15;y>=0;--y){
		for(int x=15;x>=0;--x){
			const u8 b = c->data[x][y][z];
			if(b == 0){continue;}
			const u8 sides = sideCache[x][y][z];
			if(sides&1){
				counts[0]++;
				plane[0][y][x] = b | 0x010100;
			}
			if(sides&2){
				counts[1]++;
				plane[1][y][x] = b | 0x010100;
			}
		}
		}
		chunkOptimizePlane(plane[0]);
		chunkOptimizePlane(plane[1]);
		if(counts[0] || counts[1]){
			const int cd = 1;
			for(int y=15;y>=0;--y){
			for(int x=15;x>=0;--x){
				if(plane[0][y][x]){
					const int cw = ((plane[0][y][x] >> 16) & 0xFF);
					const int ch = ((plane[0][y][x] >>  8) & 0xFF);
					const u8 b = plane[0][y][x] & 0xFF;
					chunkAddFront(c,b,x,y,z,cw,ch,cd);
				}
				if(plane[1][y][x]){
					const int cw = ((plane[1][y][x] >> 16) & 0xFF);
					const int ch = ((plane[1][y][x] >>  8) & 0xFF);
					const u8 b = plane[1][y][x] & 0xFF;
					chunkAddBack(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}

	// Top / Bottom
	for(int y=15;y>=0;--y){
		memset(plane[2],0,sizeof(plane[2]));
		memset(plane[3],0,sizeof(plane[3]));
		counts[2] = counts[3] = 0;
		for(int z=15;z>=0;--z){
		for(int x=15;x>=0;--x){
			const u8 b = c->data[x][y][z];
			if(b == 0){continue;}
			const u8 sides = sideCache[x][y][z];
			if(sides&4){
				counts[2]++;
				plane[2][z][x] = b | 0x010100;
			}
			if(sides&8){
				counts[3]++;
				plane[3][z][x] = b | 0x010100;
			}
		}
		}
		chunkOptimizePlane(plane[2]);
		chunkOptimizePlane(plane[3]);
		if(counts[2] || counts[3]){
			const int ch = 1;
			for(int z=15;z>=0;--z){
			for(int x=15;x>=0;--x){
				if(plane[2][z][x]){
					const int cw = ((plane[2][z][x] >> 16) & 0xFF);
					const int cd = ((plane[2][z][x] >>  8) & 0xFF);
					const u8 b = plane[2][z][x] & 0xFF;
					chunkAddTop(c,b,x,y,z,cw,ch,cd);
				}
				if(plane[3][z][x]){
					const int cw = ((plane[3][z][x] >> 16) & 0xFF);
					const int cd = ((plane[3][z][x] >>  8) & 0xFF);
					const u8 b = plane[3][z][x] & 0xFF;
					chunkAddBottom(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}

	// Right / Left
	for(int x=15;x>=0;--x){
		memset(plane[4],0,sizeof(plane[4]));
		memset(plane[5],0,sizeof(plane[5]));
		counts[4] = counts[5] = 0;
		for(int y=15;y>=0;--y){
		for(int z=15;z>=0;--z){
			const u8 b = c->data[x][y][z];
			if(b == 0){continue;}
			const u8 sides = sideCache[x][y][z];
			if(sides&16){
				counts[4]++;
				plane[4][y][z] = b | 0x010100;
			}
			if(sides&32){
				counts[5]++;
				plane[5][y][z] = b | 0x010100;
			}
		}
		}
		chunkOptimizePlane(plane[4]);
		chunkOptimizePlane(plane[5]);
		if(counts[4] || counts[5]){
			const int cw = 1;
			for(int y=15;y>=0;--y){for(int z=15;z>=0;--z){
				if(plane[4][y][z]){
					const int ch = ((plane[4][y][z] >>  8) & 0xFF);
					const int cd = ((plane[4][y][z] >> 16) & 0xFF);
					const u8 b = plane[4][y][z] & 0xFF;
					chunkAddRight(c,b,x,y,z,cw,ch,cd);
				}
				if(plane[5][y][z]){
					const int ch = ((plane[5][y][z] >>  8) & 0xFF);
					const int cd = ((plane[5][y][z] >> 16) & 0xFF);
					const u8 b = plane[5][y][z] & 0xFF;
					chunkAddLeft(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}
	chunkFinish(c);

	PROFILE_STOP();
}

void chunkBox(chunk *c, u16 x,u16 y,u16 z,u16 gx,u16 gy,u16 gz,u8 block){
	for(int cx=x;cx<gx;cx++){
	for(int cy=y;cy<gy;cy++){
	for(int cz=z;cz<gz;cz++){
		c->data[cx][cy][cz] = block;
	}
	}
	}
	c->dataCount |= 0x8000;
	if(( x&0xF) == 0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if(( y&0xF) == 0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if(( z&0xF) == 0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((gx&0xF) == 0xF){worldSetChunkUpdated(x+1,y  ,z  );}
	if((gy&0xF) == 0xF){worldSetChunkUpdated(x  ,y+1,z  );}
	if((gz&0xF) == 0xF){worldSetChunkUpdated(x  ,y  ,z+1);}
}

void chunkSetB(chunk *c,u16 x,u16 y,u16 z,u8 block){
	c->data[x&0xF][y&0xF][z&0xF] = block;
	c->dataCount |= 0x8000;
	if((x&0xF) == 0x0){worldSetChunkUpdated(x-1,y  ,z  );}
	if((x&0xF) == 0xF){worldSetChunkUpdated(x+1,y  ,z  );}
	if((y&0xF) == 0x0){worldSetChunkUpdated(x  ,y-1,z  );}
	if((y&0xF) == 0xF){worldSetChunkUpdated(x  ,y+1,z  );}
	if((z&0xF) == 0x0){worldSetChunkUpdated(x  ,y  ,z-1);}
	if((z&0xF) == 0xF){worldSetChunkUpdated(x  ,y  ,z+1);}
}

void chunkDraw(chunk *c, float d){
	if(c == NULL){return;}
	if(c->dataCount & 0x8000){ chunkGenMesh(c); }
	if(!c->vao){ return; }
	if(d > (fadeoutStartDistance)){
		shaderAlpha(sBlockMesh,(1.f-((d-(fadeoutStartDistance))/fadeoutDistance)));
	}else{
		shaderAlpha(sBlockMesh,1.f);
	}
	shaderTransform(sBlockMesh,c->x,c->y,c->z);

	glBindVertexArray(c->vao);
	glDrawArrays(GL_TRIANGLES,0,(c->dataCount&0x7FFF)*6);
	vboTrisCount += (c->dataCount&0x7FFF)*2;
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
	chnk->dataCount |= 0x8000;
}
