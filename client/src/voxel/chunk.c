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

void chunkInit(){
	chunkList = malloc(sizeof(chunk) * CHUNK_COUNT);
}

uint    chunkGetFree()               { return chunkFreeCount;           }
uint    chunkGetActive()             { return chunkCount;               }
uint    chunkGetGeneratedThisFrame() { return chunksGeneratedThisFrame; }
void    chunkResetCounter()          { chunksGeneratedThisFrame = 0;    }

void chunkOptimizePlane(u32 plane[CHUNK_SIZE][CHUNK_SIZE]){
	for(int y=15;y>=0;y--){
		for(int x=14;x>=0;x--){
			if(plane[x][y] && ((plane[x][y] & 0xFF) == (plane[x+1][y] & 0xFF)) && (((plane[x][y]>>16) & 0xFF) == ((plane[x+1][y]>>16)&0xFF))){
				plane[x][y] += plane[x+1][y] & 0xFF00;
				plane[x+1][y] = 0;
			}
		}
	}
	for(int x=15;x>=0;x--){
		for(int y=14;y>=0;y--){
			if(plane[x][y] && ((plane[x][y]&0xFF) == (plane[x][y+1]&0xFF)) && (((plane[x][y]>>8)&0xFF) == ((plane[x][y+1]>>8)&0xFF))){
				plane[x][y] += plane[x][y+1]&0xFF0000;
				plane[x][y+1] = 0;
			}
		}
	}
}

u8 chunkGetSides(int x,int y,int z,u8 *d,int size) {
	const int tize = size*size;
	u8 sides = 0;
	u8 *base = d + z + (y*size) + (x*tize);

	if((z == size-1) || (base[    1] == 0)){ sides |=  1;}
	if((z == 0     ) || (base[   -1] == 0)){ sides |=  2;}
	if((y == size-1) || (base[ size] == 0)){ sides |=  4;}
	if((y == 0     ) || (base[-size] == 0)){ sides |=  8;}
	if((x == size-1) || (base[ tize] == 0)){ sides |= 16;}
	if((x == 0     ) || (base[-tize] == 0)){ sides |= 32;}

	return sides;
}

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
	if(c->vbo){glDeleteBuffers(1,&c->vbo);}
	c->nextFree = chunkFirstFree;
	chunkFirstFree = c;
}

static inline void chunkFinish(chunk *c){
	if(!c->vbo) { glGenBuffers(1,&c->vbo); }
	glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
	glBufferData(GL_ARRAY_BUFFER, c->dataCount*(6*sizeof(vertexTiny)), blockMeshBuffer, GL_STATIC_DRAW);
}

static inline void chunkAddVert(chunk *c, int i, u8 x,u8 y,u8 z,u8 f, u8 u, u8 v, u8 w){
	blockMeshBuffer[c->dataCount * 6 + i] = (vertexTiny){x,y,z,u,v,w,f};
}
void chunkAddFront(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[0];
	chunkAddVert(c,0, x  ,y  ,z+d,2,0,h,bt);
	chunkAddVert(c,1, x+w,y  ,z+d,2,w,h,bt);
	chunkAddVert(c,2, x+w,y+h,z+d,2,w,0,bt);
	chunkAddVert(c,3, x+w,y+h,z+d,2,w,0,bt);
	chunkAddVert(c,4, x  ,y+h,z+d,2,0,0,bt);
	chunkAddVert(c,5, x  ,y  ,z+d,2,0,h,bt);
	c->dataCount++;
}
void chunkAddBack(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)d;
	const u8 bt = blocks[b].tex[1];
	chunkAddVert(c,0, x  ,y  ,z  ,2,0,h,bt);
	chunkAddVert(c,1, x  ,y+h,z  ,2,0,0,bt);
	chunkAddVert(c,2, x+w,y+h,z  ,2,w,0,bt);
	chunkAddVert(c,3, x+w,y+h,z  ,2,w,0,bt);
	chunkAddVert(c,4, x+w,y  ,z  ,2,w,h,bt);
	chunkAddVert(c,5, x  ,y  ,z  ,2,0,h,bt);
	c->dataCount++;
}
void chunkAddTop(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[2];
	chunkAddVert(c,0, x  ,y+h,z  ,3,0,0,bt);
	chunkAddVert(c,1, x  ,y+h,z+d,3,0,d,bt);
	chunkAddVert(c,2, x+w,y+h,z+d,3,w,d,bt);
	chunkAddVert(c,3, x+w,y+h,z+d,3,w,d,bt);
	chunkAddVert(c,4, x+w,y+h,z  ,3,w,0,bt);
	chunkAddVert(c,5, x  ,y+h,z  ,3,0,0,bt);
	c->dataCount++;
}
void chunkAddBottom(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)h;
	const u8 bt = blocks[b].tex[3];
	chunkAddVert(c,0, x  ,y  ,z  ,0,0,0,bt);
	chunkAddVert(c,1, x+w,y  ,z  ,0,w,0,bt);
	chunkAddVert(c,2, x+w,y  ,z+d,0,w,d,bt);
	chunkAddVert(c,3, x+w,y  ,z+d,0,w,d,bt);
	chunkAddVert(c,4, x  ,y  ,z+d,0,0,d,bt);
	chunkAddVert(c,5, x  ,y  ,z  ,0,0,0,bt);
	c->dataCount++;
}
void chunkAddRight(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	const u8 bt = blocks[b].tex[4];
	chunkAddVert(c,0, x+w,y  ,z  ,2,0,h,bt);
	chunkAddVert(c,1, x+w,y+h,z  ,2,0,0,bt);
	chunkAddVert(c,2, x+w,y+h,z+d,2,d,0,bt);
	chunkAddVert(c,3, x+w,y+h,z+d,2,d,0,bt);
	chunkAddVert(c,4, x+w,y  ,z+d,2,d,h,bt);
	chunkAddVert(c,5, x+w,y  ,z  ,2,0,h,bt);
	c->dataCount++;
}
void chunkAddLeft(chunk *c, u8 b,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d) {
	(void)w;
	const u8 bt = blocks[b].tex[5];
	chunkAddVert(c,0, x  ,y  ,z  ,2,0,h,bt);
	chunkAddVert(c,1, x  ,y  ,z+d,2,d,h,bt);
	chunkAddVert(c,2, x  ,y+h,z+d,2,d,0,bt);
	chunkAddVert(c,3, x  ,y+h,z+d,2,d,0,bt);
	chunkAddVert(c,4, x  ,y+h,z  ,2,0,0,bt);
	chunkAddVert(c,5, x  ,y  ,z  ,2,0,h,bt);
	c->dataCount++;
}

void chunkGenMesh(chunk *c) {
	int ac,bc;
	u8 sides;
	const uint CS   = CHUNK_SIZE;
	const uint CSCS = CS*CS;
	u8 *d = (u8 *)c->data;
	static u8 sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	static u32   aplane[CHUNK_SIZE][CHUNK_SIZE];
	static u32   bplane[CHUNK_SIZE][CHUNK_SIZE];
	if(++chunksGeneratedThisFrame > MAX_CHUNKS_GEN_PER_FRAME){return;}
	c->dataCount = 0;

	for(int x=CS-1;x>=0;--x){
	for(int y=CS-1;y>=0;--y){
	for(int z=CS-1;z>=0;--z){
		if(d[(x*CSCS)+(y*CS)+z] == 0){
			sideCache[x][y][z] = 0;
		}else{
			sideCache[x][y][z] = chunkGetSides(x,y,z,(u8 *)d,CS);
		}
	}
	}
	}

	// Front / Back
	for(int z=CS-1;z>=0;--z){
		ac = bc = 0;
		memset(aplane,0,CHUNK_SIZE*CHUNK_SIZE*4);
		memset(bplane,0,CHUNK_SIZE*CHUNK_SIZE*4);
		for(int y=CS-1;y>=0;--y){
			for(int x=CS-1;x>=0;--x){
				if(!d[(x*CSCS)+(y*CS)+z]){continue;}
				sides = sideCache[x][y][z];
				if(sides&1){
					ac++;
					aplane[y][x] = d[(x*CSCS)+(y*CS)+z] | 0x010100;
				}
				if(sides&2){
					bc++;
					bplane[y][x] = d[(x*CSCS)+(y*CS)+z] | 0x010100;
				}
			}
		}
		chunkOptimizePlane(aplane);
		chunkOptimizePlane(bplane);
		if(ac || bc){
			const int cd = 1;
			for(int y=CS-1;y>=0;--y){
				for(int x=CS-1;x>=0;--x){
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
	for(int y=CS-1;y>=0;--y){
		ac = bc = 0;
		memset(aplane,0,CHUNK_SIZE*CHUNK_SIZE*4);
		memset(bplane,0,CHUNK_SIZE*CHUNK_SIZE*4);
		for(int z=CS-1;z>=0;--z){
			for(int x=CS-1;x>=0;--x){
				if(!d[(x*CSCS)+(y*CS)+z]){continue;}
				sides = sideCache[x][y][z];
				if(sides&4){
					ac++;
					aplane[z][x] = d[(x*CSCS)+(y*CS)+z] | 0x010100;
				}
				if(sides&8){
					bc++;
					bplane[z][x] = d[(x*CSCS)+(y*CS)+z] | 0x010100;
				}
			}
		}
		chunkOptimizePlane(aplane);
		chunkOptimizePlane(bplane);
		if(ac || bc){
			const int ch = 1;
			for(int z=CS-1;z>=0;--z){
				for(int x=CS-1;x>=0;--x){
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
	for(int x=CS-1;x>=0;--x){
		ac = bc = 0;
		memset(aplane,0,CHUNK_SIZE*CHUNK_SIZE*4);
		memset(bplane,0,CHUNK_SIZE*CHUNK_SIZE*4);
		for(int y=CS-1;y>=0;--y){
			for(int z=CS-1;z>=0;--z){
				if(!d[(x*CSCS)+(y*CS)+z]){continue;}
				sides = sideCache[x][y][z];
				if(sides&16){
					ac++;
					aplane[y][z] = d[(x*CSCS)+(y*CS)+z] | 0x010100;
				}
				if(sides&32){
					bc++;
					bplane[y][z] = d[(x*CSCS)+(y*CS)+z] | 0x010100;
				}
			}
		}
		chunkOptimizePlane(aplane);
		chunkOptimizePlane(bplane);
		if(ac || bc){
			const int cw = 1;
			for(int y=CS-1;y>=0;--y){
				for(int z=CS-1;z>=0;--z){
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
}

void chunkBox(chunk *c, int x,int y,int z,int gx,int gy,int gz,u8 block){
	for(int cx=x;cx<gx;cx++){
		for(int cy=y;cy<gy;cy++){
			for(int cz=z;cz<gz;cz++){
				c->data[cx][cy][cz] = block;
			}
		}
	}
	c->dataCount |= 0x8000;
}

void chunkSetB(chunk *c,int x,int y,int z,u8 block){
	c->data[x&0xF][y&0xF][z&0xF] = block;
	c->dataCount |= 0x8000;
}

void chunkDraw(chunk *c, float d){
	if(c == NULL){return;}
	if(c->dataCount & 0x8000){ chunkGenMesh(c); }
	if(!c->vbo){ return; }
	if(d > (fadeoutStartDistance)){
		shaderAlpha(sBlockMesh,(1.f-((d-(fadeoutStartDistance))/fadeoutDistance)));
	}else{
		shaderAlpha(sBlockMesh,1.f);
	}
	shaderTransform(sBlockMesh,c->x,c->y,c->z);

	glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
	glVertexAttribPointer(0, 3, GL_BYTE,          GL_FALSE, sizeof(vertexTiny), (void *)(((char *)&blockMeshBuffer[0].x) - ((char *)blockMeshBuffer)));
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)(((char *)&blockMeshBuffer[0].u) - ((char *)blockMeshBuffer)));
	glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)(((char *)&blockMeshBuffer[0].f) - ((char *)blockMeshBuffer)));
	glDrawArrays(GL_TRIANGLES,0,(c->dataCount&0x7FFF)*6);
	vboTrisCount += (c->dataCount&0x7FFF)*2;
}
