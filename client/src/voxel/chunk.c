#include "../voxel/chunk.h"

#include "../main.h"
#include "../game/blockType.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gui/gui.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
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

#define MAX_CHUNKS_GEN_PER_FRAME 32

#ifndef __EMSCRIPTEN__
#define CHUNK_COUNT (1<<18)
#else
#define CHUNK_COUNT (1<<17)
#endif
chunk *chunkList;
u8    *chunkData;

void chunkInit(){
	chunkList = malloc(sizeof(chunk) * CHUNK_COUNT);
	chunkData = malloc(CHUNK_COUNT * 4096);
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
		chunkFreeCount--;
	}
	c->x         = x & (~0xF);
	c->y         = y & (~0xF);
	c->z         = z & (~0xF);
	c->vbo       = 0;
	c->vao       = 0;
	c->dataCount = 0xFFFF;
	chunkD *cd = chunkGetData(c);
	memset(cd,0,4096);
	return c;
}

void chunkFree(chunk *c){
	if(c == NULL){return;}
	if((c < &chunkList[0]) || (c > &chunkList[CHUNK_COUNT])){
		fprintf(stderr,"WTF am I freing\n");
		return;
	}
	chunkFreeCount++;
	if(c->vbo){glDeleteBuffers(1,&c->vbo);}
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
	glBufferData(GL_ARRAY_BUFFER, c->dataCount*(6*sizeof(vertexTiny)), blockMeshBuffer, GL_STATIC_DRAW);
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

static void chunkOptimizePlane(u32 plane[16][16]){
	for(int y=15;y>=0;y--){
	for(int x=14;x>=0;x--){
		if(!plane[x][y])                                           {continue;}
		if((plane[x][y] & 0xFF00FF) != (plane[x+1][y] & 0xFF00FF)) {continue;}
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

static inline u8 chunkGetSides(u16 x,u16 y,u16 z,chunkD *cd) {
	u8 sides = 0;

	if((z == 15) || (cd->data[x][y][z+1] == 0)){sides |=  1;}
	if((z == 0 ) || (cd->data[x][y][z-1] == 0)){sides |=  2;}
	if((y == 15) || (cd->data[x][y+1][z] == 0)){sides |=  4;}
	if((y == 0 ) || (cd->data[x][y-1][z] == 0)){sides |=  8;}
	if((x == 15) || (cd->data[x+1][y][z] == 0)){sides |= 16;}
	if((x == 0 ) || (cd->data[x-1][y][z] == 0)){sides |= 32;}

	return sides;
}

static void chunkGenMesh(chunk *c) {
	PROFILE_START();

	int ac,bc;
	u8 sides;
	chunkD *d = chunkGetData(c);
	static u8 sideCache[16][16][16];
	static u32   aplane[16][16];
	static u32   bplane[16][16];
	if(++chunksGeneratedThisFrame > MAX_CHUNKS_GEN_PER_FRAME){return;}
	c->dataCount = 0;

	for(int x=15;x>=0;--x){
	for(int y=15;y>=0;--y){
	for(int z=15;z>=0;--z){
		if(d->data[x][y][z] == 0){
			sideCache[x][y][z] = 0;
		}else{
			sideCache[x][y][z] = chunkGetSides(x,y,z,d);
		}
	}
	}
	}

	// Front / Back
	for(int z=15;z>=0;--z){
		ac = bc = 0;
		memset(aplane,0,sizeof(aplane));
		memset(bplane,0,sizeof(aplane));
		for(int y=15;y>=0;--y){
		for(int x=15;x>=0;--x){
			if(d->data[x][y][z] == 0){continue;}
			sides = sideCache[x][y][z];
			if(sides&1){
				ac++;
				aplane[y][x] = d->data[x][y][z] | 0x010100;
			}
			if(sides&2){
				bc++;
				bplane[y][x] = d->data[x][y][z] | 0x010100;
			}
		}
		}
		chunkOptimizePlane(aplane);
		chunkOptimizePlane(bplane);
		if(ac || bc) {
			const int cd = 1;
			for(int y=15;y>=0;--y){
			for(int x=15;x>=0;--x){
				if(aplane[y][x]){
					const int cw = ((aplane[y][x] >> 16) & 0xFF);
					const int ch = ((aplane[y][x] >>  8) & 0xFF);
					const u8 b = aplane[y][x] & 0xFF;
					chunkAddFront(c,b,x,y,z,cw,ch,cd);
				}
				if(bplane[y][x]){
					const int cw = ((bplane[y][x] >> 16) & 0xFF);
					const int ch = ((bplane[y][x] >>  8) & 0xFF);
					const u8 b = bplane[y][x] & 0xFF;
					chunkAddBack(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}

	// Top / Bottom
	for(int y=15;y>=0;--y){
		ac = bc = 0;
		memset(aplane,0,sizeof(aplane));
		memset(bplane,0,sizeof(aplane));
		for(int z=15;z>=0;--z){
		for(int x=15;x>=0;--x){
			if(d->data[x][y][z] == 0){continue;}
			sides = sideCache[x][y][z];
			if(sides&4){
				ac++;
				aplane[z][x] = d->data[x][y][z] | 0x010100;
			}
			if(sides&8){
				bc++;
				bplane[z][x] = d->data[x][y][z] | 0x010100;
			}
		}
		}
		chunkOptimizePlane(aplane);
		chunkOptimizePlane(bplane);
		if(ac || bc){
			const int ch = 1;
			for(int z=15;z>=0;--z){
			for(int x=15;x>=0;--x){
				if(aplane[z][x]){
					const int cw = ((aplane[z][x] >> 16) & 0xFF);
					const int cd = ((aplane[z][x] >>  8) & 0xFF);
					const u8 b = aplane[z][x] & 0xFF;
					chunkAddTop(c,b,x,y,z,cw,ch,cd);
				}
				if(bplane[z][x]){
					int cw = ((bplane[z][x] >> 16) & 0xFF);
					int cd = ((bplane[z][x] >>  8) & 0xFF);
					const u8 b = bplane[z][x] & 0xFF;
					chunkAddBottom(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}

	// Right / Left
	for(int x=15;x>=0;--x){
		ac = bc = 0;
		memset(aplane,0,sizeof(aplane));
		memset(bplane,0,sizeof(aplane));
		for(int y=15;y>=0;--y){
		for(int z=15;z>=0;--z){
			if(d->data[x][y][z] == 0){continue;}
			sides = sideCache[x][y][z];
			if(sides&16){
				ac++;
				aplane[y][z] = d->data[x][y][z] | 0x010100;
			}
			if(sides&32){
				bc++;
				bplane[y][z] = d->data[x][y][z] | 0x010100;
			}
		}
		}
		chunkOptimizePlane(aplane);
		chunkOptimizePlane(bplane);
		if(ac || bc){
			const int cw = 1;
			for(int y=15;y>=0;--y){
			for(int z=15;z>=0;--z){
				if(aplane[y][z]){
					const int ch = ((aplane[y][z] >>  8) & 0xFF);
					const int cd = ((aplane[y][z] >> 16) & 0xFF);
					const u8 b = aplane[y][z] & 0xFF;
					chunkAddRight(c,b,x,y,z,cw,ch,cd);
				}
				if(bplane[y][z]){
					const int ch = ((bplane[y][z] >>  8) & 0xFF);
					const int cd = ((bplane[y][z] >> 16) & 0xFF);
					const u8 b = bplane[y][z] & 0xFF;
					chunkAddLeft(c,b,x,y,z,cw,ch,cd);
				}
			}
			}
		}
	}
	chunkFinish(c);

	PROFILE_STOP();
}

void chunkBox(chunk *c, u16 x,u16 y,u16 z,u16 gx,u16 gy,u16 gz,u8 b){
	chunkD *cd = chunkGetData(c);
	for(int cx=x; cx < gx; cx++){
	for(int cy=y; cy < gy; cy++){
	for(int cz=z; cz < gz; cz++){
		cd->data[cx][cy][cz] = b;
	}
	}
	}
	c->dataCount |= 0x8000;
}

void chunkSetB(chunk *c,u16 x,u16 y,u16 z,u8 b){
	chunkD *cd = chunkGetData(c);
	cd->data[x&0xF][y&0xF][z&0xF] = b;
	c->dataCount |= 0x8000;
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
