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
#include "blockMesh.h"

#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/shader.h"
#include "../voxel/chungus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FADE_IN_FRAMES 48

#define BLOCKMESH_FLAG_USED 1
struct blockMesh {
	union {
		struct {
			uint vao, vbo;
		};
		blockMesh *nextFree;
	};
	u16 vboSize; // size of vbo in vertices
	u8 flags;
	u16 sideIdxStart[sideMAX], sideIdxCount[sideMAX]; // offset and count of side indices within the (sub)buffer
	u16 idxCount; // total number of indices, used when all sides are drawn
};

static u32 allocatedGlBufferBytes;
static GLuint indexBuffer;


#define CUBE_FACES 6
#define INDICES_PER_FACE 6
void blockMeshInit(){
	allocatedGlBufferBytes = 0;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	gfxObjectLabel(GL_BUFFER, indexBuffer, "Chunk vertex IBO %u", 1);
	const GLsizei indexCount = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * CUBE_FACES * INDICES_PER_FACE / 2;
	u16 *indicesRaw = malloc(indexCount * sizeof(u16));
	u16 *indices = indicesRaw;
	const GLsizei indexMax = indexCount/INDICES_PER_FACE - 1;
	for(GLsizei i=0;i<indexMax;++i){
		*indices++ = i*4 + 0;
		*indices++ = i*4 + 1;
		*indices++ = i*4 + 3;

		*indices++ = i*4 + 3;
		*indices++ = i*4 + 1;
		*indices++ = i*4 + 2;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount, indicesRaw, GL_STATIC_DRAW);
	free(indicesRaw);
}

u32 blockMeshUsedBytes(){
	return allocatedGlBufferBytes;
}

#define BLOCKMESH_ALLOC_PAGE_SIZE (1<<21) // 2MB == Huge Page on most μArchs
blockMesh *blockMeshFirstFree = NULL;

static void chunkverbufAllocPage(){
	const int count = BLOCKMESH_ALLOC_PAGE_SIZE / sizeof(blockMesh);
	blockMesh *page = malloc(BLOCKMESH_ALLOC_PAGE_SIZE);
	if(page == NULL){
		fprintf(stderr,"blockMeshAllocPage OOM\n");
		exit(5);
	}
	for(int i=0;i<count;i++){
		page[i].nextFree = blockMeshFirstFree;
		blockMeshFirstFree = &page[i];
	}
}

static blockMesh *blockMeshAlloc(){
	if(blockMeshFirstFree == NULL){chunkverbufAllocPage();}
	blockMesh *ret = blockMeshFirstFree;
	blockMeshFirstFree = blockMeshFirstFree->nextFree;
	memset(ret,0,sizeof(blockMesh));
	return ret;
}

static void blockMeshFreeSingle(blockMesh *v){
	if(v == NULL){return;}
	if(v->vbo){
		const u32 vertexSize = sizeof(vertexPacked);
		glDeleteBuffers(1,&v->vbo);
		allocatedGlBufferBytes -= vertexSize * v->vboSize;
	}
	if(v->vao){glDeleteVertexArrays(1,&v->vao);}

	v->nextFree = blockMeshFirstFree;
	blockMeshFirstFree = v;
}

void blockMeshFree(chunk *c){
	blockMeshFreeSingle(c->blockVertbuf);
	blockMeshFreeSingle(c->fluidVertbuf);
	c->blockVertbuf = NULL;
	c->fluidVertbuf = NULL;
}

#define VTX_TO_IDX_COUNT(x) ((x*3)>>1)
blockMesh *blockMeshUpdate(blockMesh *v, vertexPacked *vertices, u16 sideVtxCounts[sideMAX]){
	if(v == NULL){
		v = blockMeshAlloc();
		v->flags |= BLOCKMESH_FLAG_USED;
	}

	// Compute where the geometry for each side starts and how long it is
	// in the chunk's packed vertex buffer.
	u16 vtxCount = 0;
	for(side sideIndex=0;sideIndex<sideMAX;sideIndex++){
		v->sideIdxStart[sideIndex] = VTX_TO_IDX_COUNT(vtxCount);
		v->sideIdxCount[sideIndex] = VTX_TO_IDX_COUNT(sideVtxCounts[sideIndex]);
		vtxCount += sideVtxCounts[sideIndex];
	}
	v->idxCount = VTX_TO_IDX_COUNT(vtxCount);

	if(vtxCount == 0){
		// Empty chunk, don't allocate or update anything (except the counts themselves)
		return v;
	}

	if(!v->vao){
		glGenVertexArrays(1, &v->vao);
		//gfxObjectLabel(GL_VERTEX_ARRAY, v->vao, "Chunk %d,%d,%d VAO", c->x, c->y, c->z);
	}
	glBindVertexArray(v->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	if(!v->vbo){
		glGenBuffers(1, &v->vbo);
		//gfxObjectLabel(GL_BUFFER, v->vbo, "Chunk %d,%d,%d VBO", c->x, c->y, c->z);
	}
	glBindBuffer(GL_ARRAY_BUFFER, v->vbo);

	// Upload to the GPU, doing partial updates if possible.
	const u32 vertexSize = sizeof(vertexPacked);
	if(gfxUseSubData && (vtxCount <= v->vboSize)){
		glBufferSubData(GL_ARRAY_BUFFER,0,vertexSize * vtxCount,vertices); // Todo Measure performance impact of this!
	}else{
		allocatedGlBufferBytes -= vertexSize * v->vboSize;
		glBufferData(GL_ARRAY_BUFFER,vertexSize * vtxCount,vertices,GL_STATIC_DRAW);
		allocatedGlBufferBytes += vertexSize * vtxCount;
		v->vboSize = vtxCount;
		glVertexAttribIPointer(SHADER_ATTRIDX_PACKED, 1, GL_UNSIGNED_INT, sizeof(vertexPacked), NULL);
		glEnableVertexAttribArray(SHADER_ATTRIDX_PACKED);
	}
	return v;
}

blockMesh *blockMeshFluidUpdate(blockMesh *v, vertexFluid *vertices, u16 sideVtxCounts[sideMAX]){
	if(v == NULL){
		v = blockMeshAlloc();
		v->flags |= BLOCKMESH_FLAG_USED;
	}

	// Compute where the geometry for each side starts and how long it is
	// in the chunk's packed vertex buffer.
	u16 vtxCount = 0;
	for(side sideIndex=0;sideIndex<sideMAX;sideIndex++){
		v->sideIdxStart[sideIndex] = VTX_TO_IDX_COUNT(vtxCount);
		v->sideIdxCount[sideIndex] = VTX_TO_IDX_COUNT(sideVtxCounts[sideIndex]);
		vtxCount += sideVtxCounts[sideIndex];
	}
	v->idxCount = VTX_TO_IDX_COUNT(vtxCount);

	if(vtxCount == 0){
		// Empty chunk, don't allocate or update anything (except the counts themselves)
		return v;
	}

	if(!v->vao){
		glGenVertexArrays(1, &v->vao);
		//gfxObjectLabel(GL_VERTEX_ARRAY, v->vao, "Chunk %d,%d,%d VAO", c->x, c->y, c->z);
	}
	glBindVertexArray(v->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	if(!v->vbo){
		glGenBuffers(1, &v->vbo);
		//gfxObjectLabel(GL_BUFFER, v->vbo, "Chunk %d,%d,%d VBO", c->x, c->y, c->z);
	}
	glBindBuffer(GL_ARRAY_BUFFER, v->vbo);

	// Upload to the GPU, doing partial updates if possible.
	const u32 vertexSize = sizeof(vertexFluid);
	if(gfxUseSubData && (vtxCount <= v->vboSize)){
		glBufferSubData(GL_ARRAY_BUFFER,0,vertexSize * vtxCount,vertices); // Todo Measure performance impact of this!
	}else{
		allocatedGlBufferBytes -= vertexSize * v->vboSize;
		glBufferData(GL_ARRAY_BUFFER,vertexSize * vtxCount,vertices,GL_STATIC_DRAW);
		allocatedGlBufferBytes += vertexSize * vtxCount;
		v->vboSize = vtxCount;

		glVertexAttribIPointer(SHADER_ATTRIDX_POS,  3, GL_UNSIGNED_SHORT, sizeof(vertexFluid), (void *)((u8 *)&vertices[0].x - (u8 *)vertices));
		glVertexAttribIPointer(SHADER_ATTRIDX_TEX,  1, GL_UNSIGNED_BYTE,  sizeof(vertexFluid), (void *)((u8 *)&vertices[0].texture - (u8 *)vertices));
		glVertexAttribIPointer(SHADER_ATTRIDX_FLAG, 1, GL_UNSIGNED_BYTE,  sizeof(vertexFluid), (void *)((u8 *)&vertices[0].sideLight - (u8 *)vertices));
		glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
		glEnableVertexAttribArray(SHADER_ATTRIDX_TEX);
		glEnableVertexAttribArray(SHADER_ATTRIDX_FLAG);
	}
	return v;
}

void blockMeshDrawOne(sideMask mask, blockMesh *v){
	if(!v || (v->vao == 0) || (v->idxCount == 0)){return;}

	glBindVertexArray(v->vao);

	if(mask == sideMaskALL){
		glDrawElements(GL_TRIANGLES, v->idxCount, GL_UNSIGNED_SHORT, (const void*const*)0);
		vboTrisCount += v->idxCount / 3;
		drawCallCount++;
	}else{
		uintptr_t first[sideMAX];
		GLsizei count[sideMAX];
		uint index = 0;
		bool reuseLastSide = false;

		for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
			if(!(mask & (1 << sideIndex))){
				reuseLastSide = false;
				continue;
			}
			const uint cCount = v->sideIdxCount[sideIndex];
			if(cCount == 0){continue;}
			vboTrisCount += cCount / 3;
			const uint cFirst = v->sideIdxStart[sideIndex] * sizeof(u16);

			if(reuseLastSide){
				count[index-1] += cCount;
			}else{
				first[index] = cFirst;
				count[index] = cCount;
				index++;
				reuseLastSide = true;
			}
		}
		if(glIsMultiDrawAvailable){
			glMultiDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const void * const *)first, index);
			drawCallCount++;
		}else{
			for(uint i=0;i<index;i++){
				glDrawElements(GL_TRIANGLES, count[i], GL_UNSIGNED_SHORT, (const void*const*)first[i]);
			}
			drawCallCount+=index;
		}
	}
}
