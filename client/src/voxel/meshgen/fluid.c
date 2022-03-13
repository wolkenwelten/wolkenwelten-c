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
#include "fluid.h"
#include "../../../../common/src/misc/profiling.h"
#include "../../../../common/src/misc/side.h"
#include "../../game/light.h"
#include "../chunkvertbuf.h"
#include "../chunk.h"
#include "../bigchungus.h"
#include "block.h"
#include "shared.h"

#include <string.h>

vertexFluid fluidMeshBuffer[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * CUBE_FACES * VERTICES_PER_FACE / 2];

static vertexFluid *chunkAddVert(u16 x, u16 y, u16 z, u8 t, u8 s, u8 light, vertexFluid *vp){
	const u8 flag = s | (light << 3);
	*vp = (vertexFluid){x,y,z,t,flag};
	return vp+1;
}

/*
static vertexFluid *chunkAddFront(u8 bt,u8 x,u8 y,u8 z, u8 light, u8 fluid, const u8 corners[4], vertexFluid *vp) {
	(void)fluid;
	const u8 nl = light < 3 ? 0 : light-3;
	vp =   chunkAddVert((x  )<<8,(y<<8),(z+1)<<8,bt,sideFront, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8),(z+1)<<8,bt,sideFront, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8)+corners[2],(z+1)<<8,bt,sideFront, nl, vp);
	return chunkAddVert((x  )<<8,(y<<8)+corners[1],(z+1)<<8,bt,sideFront, nl, vp);
}
static vertexFluid *chunkAddBack(u8 bt,u8 x,u8 y,u8 z, u8 light, u8 fluid, const u8 corners[4], vertexFluid *vp) {
	(void)fluid;
	const u8 nl = light < 7 ? 0 : light-7;
	vp =   chunkAddVert((x  )<<8,(y<<8),(z  )<<8,bt,sideBack, nl, vp);
	vp =   chunkAddVert((x  )<<8,(y<<8)+corners[0],(z  )<<8,bt,sideBack, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8)+corners[3],(z  )<<8,bt,sideBack, nl, vp);
	return chunkAddVert((x+1)<<8,(y<<8),(z  )<<8,bt,sideBack, nl, vp);
}
static vertexFluid *chunkAddBottom(u8 bt,u8 x,u8 y,u8 z, u8 light, u8 fluid, vertexFluid *vp) {
	(void)fluid;
	const u8 nl = light < 9 ? 0 : light-9;
	vp =   chunkAddVert((x  )<<8,(y<<8),(z  )<<8,bt,sideBottom, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8),(z  )<<8,bt,sideBottom, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8),(z+1)<<8,bt,sideBottom, nl, vp);
	return chunkAddVert((x  )<<8,(y<<8),(z+1)<<8,bt,sideBottom, nl, vp);
}
static vertexFluid *chunkAddLeft(u8 bt,u8 x,u8 y,u8 z, u8 light, u8 fluid, const u8 corners[4], vertexFluid *vp) {
	(void)fluid;
	const u8 nl = light < 1 ? 0 : light-1;
	vp =   chunkAddVert((x  )<<8,(y<<8),(z  )<<8,bt,sideLeft, nl, vp);
	vp =   chunkAddVert((x  )<<8,(y<<8),(z+1)<<8,bt,sideLeft, nl, vp);
	vp =   chunkAddVert((x  )<<8,(y<<8)+corners[1],(z+1)<<8,bt,sideLeft, nl, vp);
	return chunkAddVert((x  )<<8,(y<<8)+corners[0],(z  )<<8,bt,sideLeft, nl, vp);
}
static vertexFluid *chunkAddRight(u8 bt,u8 x,u8 y,u8 z, u8 light, u8 fluid, const u8 corners[4], vertexFluid *vp) {
	(void)fluid;
	const u8 nl = light < 5 ? 0 : light-5;
	vp =   chunkAddVert((x+1)<<8,(y<<8),(z  )<<8,bt,sideRight, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8)+corners[3],(z  )<<8,bt,sideRight, nl, vp);
	vp =   chunkAddVert((x+1)<<8,(y<<8)+corners[2],(z+1)<<8,bt,sideRight, nl, vp);
	return chunkAddVert((x+1)<<8,(y<<8),(z+1)<<8,bt,sideRight, nl, vp);
}
*/
static void chunkPopulateFluidData(blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff){
	if((c == NULL) || (c->fluid == NULL)){return;}
	for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
	for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
	for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
		b[x][y][z] = c->fluid->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}

static int chunkFluidCalcEdge(const u8 fluidData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], const u8 blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], int *count, u8 x, u8 y, u8 z){
	if(blockData[x][y][z]){
		return 0;
	}else{
		*count = *count + 1;
		return fluidData[x][y][z] & 0xF0;
	}
}

void chunkGenFluidMesh(chunk *c){
	if((c->fluid == NULL) || ((c->flags & CHUNK_FLAG_FLUID_DIRTY) == 0)){return;}
	if(c->light == NULL){lightGen(c);}

	PROFILE_START();
	static u8       fluidData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	static u8       blockData[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2];
	u16 meshSideCounts[sideMAX];
	const u8 fluidTexture = 34;
	++chunksGeneratedThisFrame;
	memset(fluidData, 0,sizeof(fluidData)); // ToDo: Remove this!
	memset(blockData, 0,sizeof(fluidData)); // ToDo: Remove this!
	chunkPopulateFluidData(fluidData,c,1,1,1);
	chunkPopulateFluidData(fluidData,worldGetChunk(c->x-CHUNK_SIZE,c->y,c->z),1-CHUNK_SIZE,1,1);
	chunkPopulateFluidData(fluidData,worldGetChunk(c->x+CHUNK_SIZE,c->y,c->z),1+CHUNK_SIZE,1,1);
	chunkPopulateFluidData(fluidData,worldGetChunk(c->x,c->y-CHUNK_SIZE,c->z),1,1-CHUNK_SIZE,1);
	chunkPopulateFluidData(fluidData,worldGetChunk(c->x,c->y+CHUNK_SIZE,c->z),1,1+CHUNK_SIZE,1);
	chunkPopulateFluidData(fluidData,worldGetChunk(c->x,c->y,c->z-CHUNK_SIZE),1,1,1-CHUNK_SIZE);
	chunkPopulateFluidData(fluidData,worldGetChunk(c->x,c->y,c->z+CHUNK_SIZE),1,1,1+CHUNK_SIZE);

	chunkPopulateBlockData(blockData,c,1,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x-CHUNK_SIZE,c->y,c->z),1-CHUNK_SIZE,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x+CHUNK_SIZE,c->y,c->z),1+CHUNK_SIZE,1,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y-CHUNK_SIZE,c->z),1,1-CHUNK_SIZE,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y+CHUNK_SIZE,c->z),1,1+CHUNK_SIZE,1);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z-CHUNK_SIZE),1,1,1-CHUNK_SIZE);
	chunkPopulateBlockData(blockData,worldGetChunk(c->x,c->y,c->z+CHUNK_SIZE),1,1,1+CHUNK_SIZE);

	static vec vertOffsets[CHUNK_SIZE+1][CHUNK_SIZE+1][CHUNK_SIZE+1][2];

	vertexFluid *vp  = fluidMeshBuffer;
	vertexFluid *lvp = fluidMeshBuffer;

	/*
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->fluid->data[x][y][z]; // ToDo: Why does this not use fluidData???
			if((b == 0) || ( c->block && (c->block->data[x][y][z]))){continue;}
			if(sideCache[x][y][z] &sideMaskFront){
				found = true;
				plane[y][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			//chunkOptimizePlane(plane);
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 24));
				const u8 fluid = plane[y][x];
				const u8 corners[4] = {cornerHeights[x][y][z],
					cornerHeights[x][y][z+1],
					cornerHeights[x+1][y][z],
					cornerHeights[x+1][y][z+1]};
				vp = chunkAddFront(fluidTexture,x,y,z,cl,fluid,corners,vp);
			}
			}
		}
	}
	*/


	/*
	for(int z=CHUNK_SIZE-1;z>=0;--z){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->fluid->data[x][y][z];
			if((b == 0) || (c->block && (c->block->data[x][y][z]))){continue;}
			if(sideCache[x][y][z] & sideMaskBack){
				found = true;
				plane[y][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			//chunkOptimizePlane(plane);
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[y][x]){continue;}
				const int cl = ((plane[y][x] >> 24));
				const u8 fluid = plane[y][x];
				const u8 corners[4] = {cornerHeights[x][y][z],
					cornerHeights[x][y][z+1],
					cornerHeights[x+1][y][z],
					cornerHeights[x+1][y][z+1]};
				vp = chunkAddBack(fluidTexture,x,y,z,cl,fluid,corners,vp);
			}
			}
		}
	}
	*/

	for(int x=0;x<CHUNK_SIZE+1;x++){
	for(int y=0;y<CHUNK_SIZE+1;y++){
	for(int z=0;z<CHUNK_SIZE+1;z++){
			{ // Front
				int edges = 0;
				int acc  = chunkFluidCalcEdge(fluidData, blockData, &edges, x, y, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x  , y+1, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z+1);
				vertOffsets[x][y][z][0].z = edges ? MIN(255,(acc / edges)) : 0;
				if(blockData[x][y][z+1]){
					vertOffsets[x][y][z][0].z = 255;
				}
			}
			{ // Back
				int edges = 0;
				int acc  = chunkFluidCalcEdge(fluidData, blockData, &edges, x, y, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x  , y+1, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z+1);
				vertOffsets[x][y][z][1].z = edges ? MIN(255,(acc / edges)) : 0;
				if(blockData[x][y][z]){
					vertOffsets[x][y][z][0].z = 255;
				}
			}
			{ // Top
				int edges = 0;
				int acc  = chunkFluidCalcEdge(fluidData, blockData, &edges, x    , y+1, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x  , y+1, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z+1);
				vertOffsets[x][y][z][0].y = edges ? MIN(255,(acc / edges)) : 0;
			}
			{ // Bottom
				int edges = 0;
				int acc  = chunkFluidCalcEdge(fluidData, blockData, &edges, x    , y+1, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x  , y+1, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z+1);
				vertOffsets[x][y][z][1].y = edges ? MIN(255,(acc / edges)) : 0;
			}
			{ // Left
				int edges = 0;
				int acc  = chunkFluidCalcEdge(fluidData, blockData, &edges, x+1    , y, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y, z+1);
				vertOffsets[x][y][z][0].x = edges ? MIN(255,(acc / edges)) : 0;
			}
			{ // Right
				int edges = 0;
				int acc  = chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z  );
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y+1, z+1);
				acc += chunkFluidCalcEdge(fluidData, blockData, &edges, x+1, y, z+1);
				vertOffsets[x][y][z][1].x = edges ? MIN(255,(acc / edges)) : 0;
			}
		}
		}
	}

	meshSideCounts[sideFront] = vp - lvp;
	lvp = vp;
	meshSideCounts[sideBack] = vp - lvp;
	lvp = vp;
	for(int x=0;x<CHUNK_SIZE;x++){
	for(int y=0;y<CHUNK_SIZE;y++){
	for(int z=0;z<CHUNK_SIZE;z++){
		const u8 fluid = fluidData[x+1][y+1][z+1];
		if(fluid){
			const int cl = c->light->data[x][y][z];
			const u8 nl  = MAX(cl-3, 0);
			vp = chunkAddVert(x * 256, y * 256 + vertOffsets[x][y][z][0].y ,z * 256 + (vertOffsets[x][y][z][0].z / 2)   , fluidTexture, sideTop, nl, vp);
			vp = chunkAddVert(x * 256, y * 256 + vertOffsets[x][y][z+1][0].y ,z * 256 + 256 - (vertOffsets[x][y][z+1][1].z / 2), fluidTexture, sideTop, nl, vp);
			vp = chunkAddVert(x * 256 + 256,y * 256 + vertOffsets[x+1][y][z+1][0].y  ,z * 256 + 256 - (vertOffsets[x][y][z+1][1].z / 2), fluidTexture, sideTop, nl, vp);
			vp = chunkAddVert(x * 256 + 256,y * 256 + vertOffsets[x+1][y][z][0].y ,z * 256 + (vertOffsets[x][y][z][0].z / 2)    , fluidTexture, sideTop, nl, vp);
		}
	}
	}
	}
	meshSideCounts[sideTop] = vp - lvp;
	lvp = vp;
	meshSideCounts[sideBottom] = vp - lvp;
	lvp = vp;
	meshSideCounts[sideLeft] = vp - lvp;
	lvp = vp;
	meshSideCounts[sideRight] = vp - lvp;
	lvp = vp;

	/*
	for(int y=CHUNK_SIZE-1;y>=0;--y){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int z=CHUNK_SIZE-1;z>=0;--z){
		for(int x=CHUNK_SIZE-1;x>=0;--x){
			const blockId b = c->fluid->data[x][y][z];
			if((b == 0) || (c->block && (c->block->data[x][y][z]))){continue;}
			if(sideCache[x][y][z] & sideMaskBottom){
				found = true;
				plane[z][x] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			//chunkOptimizePlane(plane);
			for(int z=CHUNK_SIZE-1;z>=0;--z){
			for(int x=CHUNK_SIZE-1;x>=0;--x){
				if(!plane[z][x]){continue;}
				const int cl = ((plane[z][x] >> 24));
				const u8 fluid = plane[z][x];
				vp = chunkAddBottom(fluidTexture,x,y,z,cl,fluid,vp);
			}
			}
		}
	}
	*/


	/*
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = c->fluid->data[x][y][z];
			if((b == 0) || (c->block && (c->block->data[x][y][z]))){continue;}
			if(sideCache[x][y][z] & sideMaskRight){
				found = true;
				plane[y][z] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			//chunkOptimizePlane(plane);
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 24));
				const u8 fluid = plane[y][z];
				const u8 corners[4] = {cornerHeights[x][y][z],
					cornerHeights[x][y][z+1],
					cornerHeights[x+1][y][z],
					cornerHeights[x+1][y][z+1]};
				vp = chunkAddLeft(fluidTexture,x,y,z,cl,fluid,corners,vp);
			}
			}
		}
	}
	*/


	/*
	for(int x=CHUNK_SIZE-1;x>=0;--x){
		bool found = false;
		memset(plane,0,sizeof(plane));
		for(int y=CHUNK_SIZE-1;y>=0;--y){
		for(int z=CHUNK_SIZE-1;z>=0;--z){
			const blockId b = c->fluid->data[x][y][z];
			if((b == 0) || (c->block && (c->block->data[x][y][z]))){continue;}
			if(sideCache[x][y][z] & sideMaskLeft){
				found = true;
				plane[y][z] = (c->light->data[x][y][z] << 24) | b | 0x010100;
			}
		}
		}
		if(found){
			//chunkOptimizePlane(plane);
			for(int y=CHUNK_SIZE-1;y>=0;--y){
			for(int z=CHUNK_SIZE-1;z>=0;--z){
				if(!plane[y][z]){continue;}
				const int cl = ((plane[y][z] >> 24));
				const u8 fluid = plane[y][z];
				const u8 corners[4] = {cornerHeights[x][y][z],
					cornerHeights[x][y][z+1],
					cornerHeights[x+1][y][z],
					cornerHeights[x+1][y][z+1]};
				vp = chunkAddRight(fluidTexture,x,y,z,cl,fluid,corners,vp);
			}
			}
		}
	}
	*/

	c->flags &= ~CHUNK_FLAG_FLUID_DIRTY;
	c->fluidVertbuf = chunkvertbufFluidUpdate(NULL, fluidMeshBuffer, meshSideCounts);
	c->framesSkipped = 0;

	PROFILE_STOP();
}
