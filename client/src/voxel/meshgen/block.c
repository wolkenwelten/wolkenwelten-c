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
#include "block.h"

#include "../../../../common/src/misc/profiling.h"
#include "../../../../common/src/misc/side.h"
#include "../../game/light.h"
#include "../bigchungus.h"
#include "../chunk.h"
#include "../chunkvertbuf.h"
#include "shared.h"

#include <string.h>

vertexPacked blockMeshBuffer[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * CUBE_FACES * VERTICES_PER_FACE / 2];

static vertexPacked *chunkAddFront(u8 bt,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d, u8 light, vertexPacked *vp) {
	const u8 nl = light < 3 ? 0 : light-3;
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideFront, nl);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideFront, nl);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideFront, nl);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideFront, nl);
	return vp;
}
static vertexPacked *chunkAddBack(u8 bt,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d, u8 light, vertexPacked *vp) {
	(void)d;
	const u8 nl = light < 7 ? 0 : light-7;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideBack, nl);
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideBack, nl);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideBack, nl);
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideBack, nl);
	return vp;
}
static vertexPacked *chunkAddTop(u8 bt,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d, u8 light, vertexPacked *vp) {
	const u8 nl = light < 3 ? 0 : light-3;
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideTop, nl);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideTop, nl);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideTop, nl);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideTop, nl);
	return vp;
}
static vertexPacked *chunkAddBottom(u8 bt,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d, u8 light, vertexPacked *vp) {
	(void)h;
	const u8 nl = light < 9 ? 0 : light-9;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideBottom, nl);
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideBottom, nl);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideBottom, nl);
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideBottom, nl);
	return vp;
}
static vertexPacked *chunkAddLeft(u8 bt,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d, u8 light, vertexPacked *vp) {
	(void)w;
	const u8 nl = light < 1 ? 0 : light-1;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideLeft, nl);
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideLeft, nl);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideLeft, nl);
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideLeft, nl);
	return vp;
}
static vertexPacked *chunkAddRight(u8 bt,u8 x,u8 y,u8 z, u8 w, u8 h, u8 d, u8 light, vertexPacked *vp) {
	const u8 nl = light < 5 ? 0 : light-5;
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideRight, nl);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideRight, nl);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideRight, nl);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideRight, nl);
	return vp;
}

void chunkPopulateBlockData(blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff){
	if((c == NULL) || (c->block == NULL)){return;}
	for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
	for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
	for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
		b[x][y][z] = c->block->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}

void chunkGenBlockMesh(chunk *c){
	if((c->block == NULL) || ((c->flags & CHUNK_FLAG_DIRTY) == 0)){return;}
	lightGen(c);

	PROFILE_START();
	static blockId  blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static sideMask sideCache[CHUNK_SIZE  ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	static u32          plane[CHUNK_SIZE  ][CHUNK_SIZE  ];
	++chunksGeneratedThisFrame;
	u16 blockMeshSideCounts[sideMAX];
	memset(blockData, 0,sizeof(blockData)); // ToDo: Remove this!
	chunkPopulateBlockData(blockData,c,1,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x-CHUNK_SIZE,c->y,c->z),1-CHUNK_SIZE,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x+CHUNK_SIZE,c->y,c->z),1+CHUNK_SIZE,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y-CHUNK_SIZE,c->z),1,1-CHUNK_SIZE,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y+CHUNK_SIZE,c->z),1,1+CHUNK_SIZE,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z-CHUNK_SIZE),1,1,1-CHUNK_SIZE);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z+CHUNK_SIZE),1,1,1+CHUNK_SIZE);
	vertexPacked *vp = blockMeshBuffer;
	vertexPacked *lvp = blockMeshBuffer;

	for(int x=CHUNK_SIZE-1;x>=0;--x){
	for(int y=CHUNK_SIZE-1;y>=0;--y){
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		sideCache[x][y][z] = c->block->data[x][y][z] == 0 ? 0 : chunkGetSides(x+1,y+1,z+1,blockData);
	}
	}
	}

	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->block->data[x][y][z]; // ToDo: Why does this not use blockData???
			if(b == 0){continue;}
			if(sideCache[x][y][z] &sideMaskFront){
				found = true;
				plane[y][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 24));
				const int cw = ((plane[y][x] >> 16) & 0xF);
				const int ch = ((plane[y][x] >>  8) & 0xF);
				const blockId b = plane[y][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddFront(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideFront] = vp - lvp;
	lvp = vp;

	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->block->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskBack){
				found = true;
				plane[y][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 24));
				const int cw = ((plane[y][x] >> 16) & 0xF);
				const int ch = ((plane[y][x] >>  8) & 0xF);
				const blockId b = plane[y][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddBack(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideBack] = vp - lvp;
	lvp = vp;

	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->block->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskTop){
				found = true;
				plane[z][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 24));
				const int cw = ((plane[z][x] >> 16) & 0xF);
				const int cd = ((plane[z][x] >>  8) & 0xF);
				const blockId b = plane[z][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddTop(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideTop] = vp - lvp;
	lvp = vp;

	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->block->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskBottom){
				found = true;
				plane[z][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 24));
				const int cw = ((plane[z][x] >> 16) & 0xF);
				const int cd = ((plane[z][x] >>  8) & 0xF);
				const blockId b = plane[z][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddBottom(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideBottom] = vp - lvp;
	lvp = vp;

	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = c->block->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskRight){
				found = true;
				plane[y][z] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 24));
				const int ch = ((plane[y][z] >>  8) & 0xF);
				const int cd = ((plane[y][z] >> 16) & 0xF);
				const blockId b = plane[y][z] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddLeft(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideLeft] = vp - lvp;
	lvp = vp;

	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = c->block->data[x][y][z];
			if(b == 0){continue;}
			if(sideCache[x][y][z] & sideMaskLeft){
				found = true;
				plane[y][z] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 24));
				const int ch = ((plane[y][z] >>  8) & 0xF);
				const int cd = ((plane[y][z] >> 16) & 0xF);
				const blockId b = plane[y][z] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddRight(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideRight] = vp - lvp;
	if(!c->blockVertbuf){ c->fadeIn = FADE_IN_FRAMES; }
	c->blockVertbuf = chunkvertbufBlockUpdate(c->blockVertbuf, blockMeshBuffer, blockMeshSideCounts);
	c->flags &= ~CHUNK_FLAG_DIRTY;
	c->framesSkipped = 0;

	PROFILE_STOP();
}
