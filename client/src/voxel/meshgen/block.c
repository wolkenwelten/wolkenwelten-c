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

/* Only planes with at least that many faces vixible will be run through the
 | optimizer/greedy mesher.
 */
#define CHUNK_OPTIMIZE_THRESHOLD 8

vertexPacked blockMeshBuffer[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * CUBE_FACES * VERTICES_PER_FACE / 2];

static vertexPacked *chunkAddFront(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideFront, (light      ) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideFront, (light >>  4) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideFront, (light >>  8) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideFront, (light >> 12) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddBack(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	(void)d;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideBack, (light      ) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideBack, (light >>  4) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideBack, (light >>  8) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideBack, (light >> 12) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddTop(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideTop, (light      ) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideTop, (light >>  4) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideTop, (light >>  8) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideTop, (light >> 12) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddBottom(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	(void)h;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideBottom, (light      ) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideBottom, (light >>  4) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideBottom, (light >>  8) & 0x1F);
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideBottom, (light >> 12) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddLeft(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	(void)w;
	*vp++ = mkVertex(x  ,y  ,z  ,bt,sideLeft, (light      ) & 0x1F);
	*vp++ = mkVertex(x  ,y  ,z+d,bt,sideLeft, (light >>  4) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z+d,bt,sideLeft, (light >>  8) & 0x1F);
	*vp++ = mkVertex(x  ,y+h,z  ,bt,sideLeft, (light >> 12) & 0x1F);
	return vp;
}
static vertexPacked *chunkAddRight(u8 bt, u8 x, u8 y, u8 z, u8 w, u8 h, u8 d, u32 light, vertexPacked *vp) {
	*vp++ = mkVertex(x+w,y  ,z  ,bt,sideRight, (light      ) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z  ,bt,sideRight, (light >>  4) & 0x1F);
	*vp++ = mkVertex(x+w,y+h,z+d,bt,sideRight, (light >>  8) & 0x1F);
	*vp++ = mkVertex(x+w,y  ,z+d,bt,sideRight, (light >> 12) & 0x1F);
	return vp;
}

static void chunkPopulateLightData(u8 b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, int xoff, int yoff, int zoff){
	if(c == NULL){return;}
	for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
	for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
	for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
		b[x][y][z] = c->light->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}

static int chunkLightTopBottom(const u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], int x, int y, int z){
	const int a = lightData[x  ][y][z  ];
	const int b = lightData[x  ][y][z+1];
	const int c = lightData[x+1][y][z  ];
	const int d = lightData[x+1][y][z+1];
	return MIN((a+b+c+d)/4, 0xF);
}

static int chunkLightFrontBack(const u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], int x, int y, int z){
	const int a = lightData[x  ][y  ][z];
	const int b = lightData[x  ][y+1][z];
	const int c = lightData[x+1][y  ][z];
	const int d = lightData[x+1][y+1][z];
	return MIN((a+b+c+d)/4, 0xF);
}

static int chunkLightLeftRight(const u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], int x, int y, int z){
	const int a = lightData[x][y  ][z  ];
	const int b = lightData[x][y+1][z  ];
	const int c = lightData[x][y  ][z+1];
	const int d = lightData[x][y+1][z+1];
	return MIN((a+b+c+d)/4, 0xF);
}

static void chunkPopulateSideCache(sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2]){
	for(int x=CHUNK_SIZE-1;x>=0;--x){
	for(int y=CHUNK_SIZE-1;y>=0;--y){
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		sideCache[x][y][z] = blockData[x+1][y+1][z+1] == 0 ? 0 : chunkGetSides(x+1,y+1,z+1,blockData);
	}
	}
	}
}

static void chunkPopulateBlockAndLightData(chunk *c, blockId  blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2]){
	for(int x=-1;x<2;x++){
	for(int y=-1;y<2;y++){
	for(int z=-1;z<2;z++){
		const int xo = CHUNK_SIZE*x;
		const int yo = CHUNK_SIZE*y;
		const int zo = CHUNK_SIZE*z;
		chunk *cc = worldGetChunk(c->x+xo,c->y+yo,c->z+zo);
		lightGen(cc);
		chunkPopulateBlockData(blockData,cc,1+xo,1+yo,1+zo);
		chunkPopulateLightData(lightData,cc,1+xo,1+yo,1+zo);
	}
	}
	}
}

static vertexPacked *chunkGenBlockMeshFront(vertexPacked *vp, blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], u16 blockMeshSideCounts[sideMAX]){
	u32 plane[CHUNK_SIZE][CHUNK_SIZE];
	vertexPacked *lvp = vp;

	/* First we slice the chunk into many, zero-initialized, planes */
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		int found = 0;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			if(sideCache[x][y][z] & sideMaskFront){
				found++;
				const u32 light = 0
					| (chunkLightFrontBack(lightData,x  ,y  ,z+2) <<  0)
					| (chunkLightFrontBack(lightData,x+1,y  ,z+2) <<  4)
					| (chunkLightFrontBack(lightData,x+1,y+1,z+2) <<  8)
					| (chunkLightFrontBack(lightData,x  ,y+1,z+2) << 12);
				plane[y][x] = (light << 16) | 0x1100 | blockData[x+1][y+1][z+1];
			}
		}
		}
		if(found){
			if(found > CHUNK_OPTIMIZE_THRESHOLD){
				chunkOptimizePlane(plane);
			}
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 16) & 0xFFFF);
				const int cw = ((plane[y][x] >> 12) &    0xF);
				const int ch = ((plane[y][x] >>  8) &    0xF);
				const blockId b = plane[y][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideFront];
				vp = chunkAddFront(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideFront] = vp - lvp;
	return vp;
}

static vertexPacked *chunkGenBlockMeshBack(vertexPacked *vp, blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], u16 blockMeshSideCounts[sideMAX]){
	u32 plane[CHUNK_SIZE][CHUNK_SIZE];
	vertexPacked *lvp = vp;

	for(int z=CHUNK_SIZE-1;z>=0;--z){
		int found = 0;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			if(sideCache[x][y][z] & sideMaskBack){
				found++;
				const u32 light = 0
					| (chunkLightFrontBack(lightData,x  ,y  ,z) <<  0)
					| (chunkLightFrontBack(lightData,x  ,y+1,z) <<  4)
					| (chunkLightFrontBack(lightData,x+1,y+1,z) <<  8)
					| (chunkLightFrontBack(lightData,x+1,y  ,z) << 12);
				plane[y][x] = (light << 16) | 0x1100 | blockData[x+1][y+1][z+1];
			}
		}
		}
		if(found){
			if(found > CHUNK_OPTIMIZE_THRESHOLD){
				chunkOptimizePlane(plane);
			}
			const int cd = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 16) & 0xFFFF);
				const int cw = ((plane[y][x] >> 12) &    0xF);
				const int ch = ((plane[y][x] >>  8) &    0xF);
				const blockId b = plane[y][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideBack];
				vp = chunkAddBack(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideBack] = vp - lvp;
	return vp;
}

static vertexPacked *chunkGenBlockMeshTop(vertexPacked *vp, blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], u16 blockMeshSideCounts[sideMAX]){
	u32 plane[CHUNK_SIZE][CHUNK_SIZE];
	vertexPacked *lvp = vp;

	for(int y=CHUNK_SIZE-1;y>=0;--y){
		int found = 0;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			if(sideCache[x][y][z] & sideMaskTop){
				found++;
				const u32 light = 0
				  | (chunkLightTopBottom(lightData,x  ,y+2,z  ) <<  0)
				  | (chunkLightTopBottom(lightData,x  ,y+2,z+1) <<  4)
				  | (chunkLightTopBottom(lightData,x+1,y+2,z+1) <<  8)
				  | (chunkLightTopBottom(lightData,x+1,y+2,z  ) << 12);
				plane[z][x] = (light << 16) | 0x1100 | blockData[x+1][y+1][z+1];
			}
		}
		}
		if(found){
			if(found > CHUNK_OPTIMIZE_THRESHOLD){
				chunkOptimizePlane(plane);
			}
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 16) & 0xFFFF);
				const int cw = ((plane[z][x] >> 12) &    0xF);
				const int cd = ((plane[z][x] >>  8) &    0xF);
				const blockId b = plane[z][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideTop];
				vp = chunkAddTop(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideTop] = vp - lvp;
	return vp;
}

static vertexPacked *chunkGenBlockMeshBottom(vertexPacked *vp, blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], u16 blockMeshSideCounts[sideMAX]){
	u32 plane[CHUNK_SIZE][CHUNK_SIZE];
	vertexPacked *lvp = vp;

	for(int y=CHUNK_SIZE-1;y>=0;--y){
		int found = 0;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			if(sideCache[x][y][z] & sideMaskBottom){
				found++;
				const u32 light = 0
				  | (chunkLightTopBottom(lightData,x  ,y,z  ) << ( 0 + 16))
				  | (chunkLightTopBottom(lightData,x+1,y,z  ) << ( 4 + 16))
				  | (chunkLightTopBottom(lightData,x+1,y,z+1) << ( 8 + 16))
				  | (chunkLightTopBottom(lightData,x  ,y,z+1) << (12 + 16));
				plane[z][x] = light | 0x1100 | blockData[x+1][y+1][z+1];
			}
		}
		}
		if(found){
			if(found > CHUNK_OPTIMIZE_THRESHOLD){
				chunkOptimizePlane(plane);
			}
			const int ch = 1;
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 16) & 0xFFFF);
				const int cw = ((plane[z][x] >> 12) &    0xF);
				const int cd = ((plane[z][x] >>  8) &    0xF);
				const blockId b = plane[z][x] & 0xFF;
				const u8 bt = blocks[b].tex[sideBottom];
				vp = chunkAddBottom(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideBottom] = vp - lvp;
	return vp;
}

static vertexPacked *chunkGenBlockMeshLeft(vertexPacked *vp, blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], u16 blockMeshSideCounts[sideMAX]){
	u32 plane[CHUNK_SIZE][CHUNK_SIZE];
	vertexPacked *lvp = vp;

	for(int x=CHUNK_SIZE-1;x>=0;--x){
		int found = 0;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			if(sideCache[x][y][z] & sideMaskRight){
				found++;
				const u32 light = 0
					| (chunkLightLeftRight(lightData,x,y  ,z  ) <<  0)
					| (chunkLightLeftRight(lightData,x,y  ,z+1) <<  4)
					| (chunkLightLeftRight(lightData,x,y+1,z+1) <<  8)
					| (chunkLightLeftRight(lightData,x,y+1,z  ) << 12);
				plane[y][z] = (light << 16) | 0x1100 | blockData[x+1][y+1][z+1];
			}
		}
		}
		if(found){
			if(found > CHUNK_OPTIMIZE_THRESHOLD){
				chunkOptimizePlane(plane);
			}
			const int cw = 1;
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 16) & 0xFFFF);
				const int cd = ((plane[y][z] >> 12) &    0xF);
				const int ch = ((plane[y][z] >>  8) &    0xF);
				const blockId b = plane[y][z] & 0xFF;
				const u8 bt = blocks[b].tex[sideLeft];
				vp = chunkAddLeft(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideLeft] = vp - lvp;
	return vp;
}

static vertexPacked *chunkGenBlockMeshRight(vertexPacked *vp, blockId blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], u8 lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], sideMask sideCache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], u16 blockMeshSideCounts[sideMAX]){
	u32 plane[CHUNK_SIZE][CHUNK_SIZE];
	vertexPacked *lvp = vp;

	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = blockData[x+1][y+1][z+1];
			if(b && (sideCache[x][y][z] & sideMaskLeft)){
				found = true;
				const u32 light = 0
					| (chunkLightLeftRight(lightData,x+2,y  ,z  ) <<  0)
					| (chunkLightLeftRight(lightData,x+2,y+1,z  ) <<  4)
					| (chunkLightLeftRight(lightData,x+2,y+1,z+1) <<  8)
					| (chunkLightLeftRight(lightData,x+2,y  ,z+1) << 12);
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
				const int cl = ((plane[y][z] >> 16) & 0xFFFF);
				const int cd = ((plane[y][z] >> 12) &    0xF);
				const int ch = ((plane[y][z] >>  8) &    0xF);
				const blockId b = plane[y][z] & 0xFF;
				const u8 bt = blocks[b].tex[sideRight];
				vp = chunkAddRight(bt,x,y,z,cw,ch,cd,cl,vp);
			}
			}
		}
	}
	blockMeshSideCounts[sideRight] = vp - lvp;
	return vp;
}


void chunkGenBlockMesh(chunk *c){
	if((c == NULL) || (c->block == NULL) || ((c->flags & CHUNK_FLAG_DIRTY) == 0)){return;}
	PROFILE_START();

	blockId  blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	u8       lightData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	sideMask sideCache[CHUNK_SIZE  ][CHUNK_SIZE  ][CHUNK_SIZE  ];
	u16 blockMeshSideCounts[sideMAX];
	vertexPacked  *vp = blockMeshBuffer;
	++chunksGeneratedThisFrame;

	/* First we populate our blockData and lightData buffers, this has to be done since
	 | we need to look not just at the data within our chunk, but also into every neighboring
	 | chunk, to make this more convenient and efficient we populate these buffers once in the
	 | beginning and then do calculations on those. This is also a very convenient place to handle
	 | empty chunks since during mesh generation we only see zeroes.
	 */
	chunkPopulateBlockAndLightData(c, blockData, lightData);
	/* The sideCache is a bitmask which determines which faces of a block need to be included
	 | in the mesh.
	 */
	chunkPopulateSideCache(sideCache, blockData);

	/* Now we generate all meshes, plane by plane, keeping track of the start/end positions
	 | within blockMeshSideCounts to enable rendering block planes separately
	 */
	vp = chunkGenBlockMeshFront  (vp, blockData, lightData, sideCache, blockMeshSideCounts);
	vp = chunkGenBlockMeshBack   (vp, blockData, lightData, sideCache, blockMeshSideCounts);
	vp = chunkGenBlockMeshTop    (vp, blockData, lightData, sideCache, blockMeshSideCounts);
	vp = chunkGenBlockMeshBottom (vp, blockData, lightData, sideCache, blockMeshSideCounts);
	vp = chunkGenBlockMeshLeft   (vp, blockData, lightData, sideCache, blockMeshSideCounts);
	vp = chunkGenBlockMeshRight  (vp, blockData, lightData, sideCache, blockMeshSideCounts);

	if(!c->blockVertbuf){
		c->fadeIn = FADE_IN_FRAMES;
	}
	c->blockVertbuf = chunkvertbufBlockUpdate(c->blockVertbuf, blockMeshBuffer, blockMeshSideCounts);
	c->flags &= ~CHUNK_FLAG_DIRTY;
	c->framesSkipped = 0;

	PROFILE_STOP();
}
