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
#include "../voxel/chunkvertbuf.h"

#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/shader.h"
#include "../voxel/chungus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FADE_IN_FRAMES 48

#define CHUNKVERTBUF_FLAG_USED 1
struct chunkvertbuf {
	union {
		struct {
			uint vao,vbo;
		};
		chunkvertbuf *nextFree;
	};
	u16 vboSize; // size of vbo in vertices
	u8 flags;
	u16 sideIdxStart[sideMAX],sideIdxCount[sideMAX]; // offset and count of side indices within the (sub)buffer
	u16 idxCount; // total number of indices, used when all sides are drawn
};

static u32 allocatedGlBufferBytes;
static GLuint indexBuffer;

static void setVAOFormatPacked(){
	glVertexAttribIPointer(SHADER_ATTRIDX_PACKED, 1, GL_UNSIGNED_INT, sizeof(vertexPacked), NULL);
	glEnableVertexAttribArray(SHADER_ATTRIDX_PACKED);
}

#define CUBE_FACES 6
#define INDICES_PER_FACE 6
void chunkvertbufInit(){
	allocatedGlBufferBytes = 0;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	gfxObjectLabel(GL_BUFFER, indexBuffer, "Chunk vertex IBO");
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

u32 chunkvertbufUsedBytes(){
	return allocatedGlBufferBytes;
}

#define CHUNKVERTBUF_ALLOC_PAGE_SIZE (1<<21) // 2MB == Huge Page on most μArchs
chunkvertbuf *chunkvertbufFirstFree = NULL;

static void chunkverbufAllocPage(){
	const int count = CHUNKVERTBUF_ALLOC_PAGE_SIZE / sizeof(chunkvertbuf);
	chunkvertbuf *page = malloc(CHUNKVERTBUF_ALLOC_PAGE_SIZE);
	if(page == NULL){
		fprintf(stderr,"chunkvertbufAllocPage OOM\n");
		exit(5);
	}
	for(int i=0;i<count;i++){
		page[i].nextFree = chunkvertbufFirstFree;
		chunkvertbufFirstFree = &page[i];
	}
}

static chunkvertbuf *chunkvertbufAlloc(){
	if(chunkvertbufFirstFree == NULL){chunkverbufAllocPage();}
	chunkvertbuf *ret = chunkvertbufFirstFree;
	chunkvertbufFirstFree = chunkvertbufFirstFree->nextFree;
	memset(ret,0,sizeof(chunkvertbuf));
	return ret;
}

void chunkvertbufFree(struct chunk *c){
	if(c->vertbuf == NULL){return;}
	if(c->vertbuf->vbo){
		const u32 vertexSize = sizeof(vertexPacked);
		glDeleteBuffers(1,&c->vertbuf->vbo);
		allocatedGlBufferBytes -= vertexSize * c->vertbuf->vboSize;
	}
	if(c->vertbuf->vao){glDeleteVertexArrays(1,&c->vertbuf->vao);}

	c->vertbuf->nextFree = chunkvertbufFirstFree;
	chunkvertbufFirstFree = c->vertbuf;

	c->vertbuf = NULL;
}


#define VTX_TO_IDX_COUNT(x) ((x*3)>>1)
void chunkvertbufUpdate(chunk *c, vertexPacked *vertices, u16 sideVtxCounts[sideMAX]) {
	struct chunkvertbuf *v = c->vertbuf;
	if(v == NULL) {
		c->vertbuf = v = chunkvertbufAlloc();
		v->flags |= CHUNKVERTBUF_FLAG_USED;
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
		return;
	}

	if(!v->vao){
		glGenVertexArrays(1, &v->vao);
		gfxObjectLabel(GL_VERTEX_ARRAY, v->vao, "Chunk %d,%d,%d VAO", c->x, c->y, c->z);
	}
	glBindVertexArray(v->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	if(!v->vbo){
		glGenBuffers(1, &v->vbo);
		gfxObjectLabel(GL_BUFFER, v->vbo, "Chunk %d,%d,%d VBO", c->x, c->y, c->z);
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
		setVAOFormatPacked();
	}
}

void chunkvertbufDrawOne(struct chunk *c, sideMask mask){
	struct chunkvertbuf *v = c->vertbuf;
	if(v->vao == 0 || v->idxCount == 0){return;}
	uint bufOffset = 0;

	shaderTransform(sBlockMesh,c->x-subBlockViewOffset.x,c->y-subBlockViewOffset.y,c->z-subBlockViewOffset.z);

	glBindVertexArray(v->vao);
	if(mask == sideMaskALL || !glIsMultiDrawAvailable){
		glDrawElements(GL_TRIANGLES,v->idxCount,GL_UNSIGNED_SHORT,NULL);
		vboTrisCount += v->idxCount / 3;
		drawCallCount++;
	}else{
		// We need one face less max, otherwise it would mean mask == sideMaskALL
		uintptr_t first[sideMAX - 1];
		GLsizei count[sideMAX - 1];
		uint index = 0;
		bool reuseLastSide = false;
		for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
			if(mask & (1 << sideIndex)){
				const uint cFirst = bufOffset + c->vertbuf->sideIdxStart[sideIndex];
				const uint cCount = c->vertbuf->sideIdxCount[sideIndex];
				if(cCount == 0){continue;}
				vboTrisCount += cCount / 3;
				if(reuseLastSide){
					count[index-1] += cCount;
				}else{
					// OpenGL expects a *byte* offset into the index buffer,
					// which is interpreted as GL_UNSIGNED_SHORT only afterwards
					first[index] = cFirst * sizeof(u16);
					count[index] = cCount;
					index++;
					reuseLastSide = true;
				}
			}else{
				reuseLastSide = false;
			}
		}
		glMultiDrawElements(GL_TRIANGLES,count,GL_UNSIGNED_SHORT,(const void*const*)first,index);
		drawCallCount++;
	}
}
