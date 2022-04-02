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

static vertexPacked *chunkAddFront(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideFront, (light      ) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideFront, (light >>  5) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideFront, (light >> 10) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideFront, (light >> 15) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddBack(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	(void)d;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideBack, (light      ) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideBack, (light >>  5) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideBack, (light >> 10) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideBack, (light >> 15) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddTop(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideTop, (light      ) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideTop, (light >>  5) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideTop, (light >> 10) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideTop, (light >> 15) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddBottom(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	(void)h;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideBottom, (light      ) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideBottom, (light >>  5) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideBottom, (light >> 10) & 0x1F);
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideBottom, (light >> 15) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddLeft(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	(void)w;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideLeft, (light      ) & 0x1F);
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideLeft, (light >>  5) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideLeft, (light >> 10) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideLeft, (light >> 15) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddRight(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideRight, (light      ) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideRight, (light >>  5) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideRight, (light >> 10) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideRight, (light >> 15) & 0x1F);
	return vp;
}

static void chunkPopulateLightData(u8 b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, int xoff, int yoff, int zoff){
	if(c->light == NULL){
		if(c->block){
			lightGen(c);
		}else{
			const int xd = xoff == 1 ? 0 : (xoff > 1) ? -1 : 1;
			const int yd = yoff == 1 ? 0 : (yoff > 1) ? -1 : 1;
			const int zd = zoff == 1 ? 0 : (zoff > 1) ? -1 : 1;
			const int ld = yd * 2;
			for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
			for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
			for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
				b[x][y][z] = MAX(0,MIN(b[x+xd][y+yd][z+zd] - ld, 0x1F));
			}
			}
			}
			return;
		}
	}
	for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
	for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
	for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
		b[x][y][z] = c->light->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}

static int chunkLightTopBottom(const u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], int x, int y, int z){
	return MIN(((lightData[x][y][z]
		 + lightData[x][y][z+1]
		 + lightData[x+1][y][z]
		 + lightData[x+1][y][z+1]) / 4), 31);
}

void chunkGenBlockMesh(chunk *c){
	if((c->block == NULL) || ((c->flags & CHUNK_FLAG_DIRTY) == 0)){return;}
	lightGen(c);

	PROFILE_START();
	static blockId  blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static u8       lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static sideMask sideCache[CHUNK_SIZE  ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	static u64          plane[CHUNK_SIZE  ][CHUNK_SIZE  ];
	++chunksGeneratedThisFrame;
	u16 blockMeshSideCounts[sideMAX];
	memset(blockData, 0,sizeof(blockData)); // ToDo: Remove this!

	chunkPopulateBlockData(blockData,c,1,1,1);
	chunkPopulateLightData(lightData,c,1,1,1);
	for(int x=-1;x<2;x++){
	for(int y=-1;y<2;y++){
	for(int z=-1;z<2;z++){
		if((x|y|z) == 0){continue;}
		const int xo = CHUNK_SIZE*x;
		const int yo = CHUNK_SIZE*y;
		const int zo = CHUNK_SIZE*z;
		chunk *cc = worldGetChunk(c->x+xo,c->y+yo,c->z+zo);
		chunkPopulateBlockData(blockData,cc,1+xo,1+yo,1+zo);
		chunkPopulateLightData(lightData,cc,1+xo,1+yo,1+zo);
	}
	}
	}
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
				const u64 light = lightData[x+1][y+1][z+2]
					| (lightData[x+1][y+1][z+2] <<  5)
					| (lightData[x+1][y+1][z+2] << 10)
					| (lightData[x+1][y+1][z+2] << 15);
				plane[y][x] = (light << 16) | 0x1100 | b;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 16) & 0xFFFFF);
				const int cw = ((plane[y][x] >> 12) &     0xF);
				const int ch = ((plane[y][x] >>  8) &     0xF);
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
				const u64 light = lightData[x+1][y+1][z]
					| (lightData[x+1][y+1][z] <<  5)
					| (lightData[x+1][y+1][z] << 10)
					| (lightData[x+1][y+1][z] << 15);
				plane[y][x] = (light << 16) | 0x1100 | b;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 16) & 0xFFFFF);
				const int cw = ((plane[y][x] >> 12) &     0xF);
				const int ch = ((plane[y][x] >>  8) &     0xF);
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
				const u64 light = lightData[x+1][y+2][z+1]
					| (lightData[x+1][y+2][z+1] <<  5)
					| (lightData[x+1][y+2][z+1] << 10)
					| (lightData[x+1][y+2][z+1] << 15);
				(void)chunkLightTopBottom;
				/*
				const u64 light = chunkLightTopBottom(lightData,x,y+2,z)
					| (chunkLightTopBottom(lightData,x  ,y+2,z+1) <<  5)
					| (chunkLightTopBottom(lightData,x+1,y+2,z+1) << 10)
					| (chunkLightTopBottom(lightData,x+1,y+2,z  ) << 15);
				*/
				plane[z][x] = (light << 16) | 0x1100 | b;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 16) & 0xFFFFF);
				const int cw = ((plane[z][x] >> 12) &     0xF);
				const int cd = ((plane[z][x] >>  8) &     0xF);
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
				const u64 light = lightData[x+1][y][z+1]
					| (lightData[x+1][y][z+1] <<  5)
					| (lightData[x+1][y][z+1] << 10)
					| (lightData[x+1][y][z+1] << 15);
				/*
				const u64 light = chunkLightTopBottom(lightData,x,y+1,z)
					| (chunkLightTopBottom(lightData,x  ,y+1,z+1) <<  5)
					| (chunkLightTopBottom(lightData,x+1,y+1,z+1) << 10)
					| (chunkLightTopBottom(lightData,x+1,y+1,z  ) << 15);
				*/
				plane[z][x] = (light << 16) | 0x1100 | b;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 16) & 0xFFFFF);
				const int cw = ((plane[z][x] >> 12) &     0xF);
				const int cd = ((plane[z][x] >>  8) &     0xF);
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
				const u64 light = lightData[x][y+1][z+1]
					| (lightData[x][y+1][z+1] <<  5)
					| (lightData[x][y+1][z+1] << 10)
					| (lightData[x][y+1][z+1] << 15);
				plane[y][z] = (light << 16) | 0x1100 | b;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 16) & 0xFFFFF);
				const int cd = ((plane[y][z] >> 12) &     0xF);
				const int ch = ((plane[y][z] >>  8) &     0xF);
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
				const u64 light = lightData[x+2][y+1][z+1]
					| (lightData[x+2][y+1][z+1] <<  5)
					| (lightData[x+2][y+1][z+1] << 10)
					| (lightData[x+2][y+1][z+1] << 15);
				plane[y][z] = (light << 16) | 0x1100 | b;
			}
		}
		}
		if(found){
			chunkOptimizePlane(plane);
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 16) & 0xFFFFF);
				const int cd = ((plane[y][z] >> 12) &     0xF);
				const int ch = ((plane[y][z] >>  8) &     0xF);
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
