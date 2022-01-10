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

#define CHUNKVERTBUF_FLAG_USED 1
struct chunkvertbuf {
	struct{
		// for regular GL buffers
		uint vao,vbo;
		u16 vboSize; // size of vbo in vertices
	};
	u8 flags;
	u16 sideStart[sideMAX],sideCount[sideMAX]; // offset and count of side vertices within the (sub)buffer
	u16 count; // total number of vertices, used when all sides are drawn
};

static u32 allocatedGlBufferBytes;

static uint makeVAO(){
	uint vao;
	glGenVertexArrays(1, &vao);
	return vao;
}

static void setVAOFormat(){
	glVertexAttribPointer(SHADER_ATTRIDX_POS, 3, GL_BYTE,          GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, x));
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
	glVertexAttribPointer(SHADER_ATTRIDX_TEX, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, u));
	glEnableVertexAttribArray(SHADER_ATTRIDX_TEX);
	glVertexAttribIPointer(SHADER_ATTRIDX_FLAG, 1, GL_UNSIGNED_BYTE, sizeof(vertexTiny), (void *)offsetof(vertexTiny, f));
	glEnableVertexAttribArray(SHADER_ATTRIDX_FLAG);
}

static uint makeVBO(){
	uint vbo;
	glGenBuffers(1,&vbo);
	return vbo;
}

void chunkvertbufInit(){
	allocatedGlBufferBytes = 0;
}

u32 chunkvertbufUsedBytes(){
	return allocatedGlBufferBytes;
}

u32 chunkvertbufMaxBytes(){
	return 0;
}

void chunkvertbufUpdate(chunk *c, vertexTiny *vertices, u16 sideCounts[sideMAX]) {
	struct chunkvertbuf *v = c->vertbuf;
	if(v == NULL) {
		v = calloc(1, sizeof(struct chunkvertbuf));
		if(v == NULL){
			fprintf(stderr,"Couldn't allocate chunk vertbuf, exiting!\n");
			exit(1);
		}
		v->flags |= CHUNKVERTBUF_FLAG_USED;
		c->vertbuf = v;
	}

	// Compute where the geometry for each side starts and how long it is
	// in the chunk's packed vertex buffer.
	u16 count = 0;
	for(side sideIndex=0;sideIndex<sideMAX;sideIndex++){
		v->sideStart[sideIndex] = count;
		v->sideCount[sideIndex] = sideCounts[sideIndex];
		count += sideCounts[sideIndex];
	}
	v->count = count;

	if(count == 0){
		// Empty chunk, don't allocate or update anything (except the counts themselves)
		return;
	}

	if(!v->vao){
		v->vao = makeVAO();
		if(glIsDebugAvailable){
			char name[64];
			snprintf(name, sizeof(name), "Chunk %d,%d,%d VAO", c->x, c->y, c->z);
			glObjectLabel(GL_VERTEX_ARRAY, v->vao, -1, name);
		}
	}
	glBindVertexArray(v->vao);

	if(!v->vbo){
		v->vbo = makeVBO();
		if(glIsDebugAvailable){
			char name[64];
			snprintf(name, sizeof(name), "Chunk %d,%d,%d VBO", c->x, c->y, c->z);
			glObjectLabel(GL_VERTEX_ARRAY, v->vbo, -1, name);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER,v->vbo);

	// Upload to the GPU, doing partial updates if possible.
	if(gfxUseSubData && (count <= v->vboSize)){
		glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertexTiny) * count,vertices); // Todo Measure performance impact of this!
	}else{
		allocatedGlBufferBytes -= sizeof(vertexTiny) * v->vboSize;
		glBufferData(GL_ARRAY_BUFFER,sizeof(vertexTiny) * count,vertices,GL_STATIC_DRAW);
		allocatedGlBufferBytes += sizeof(vertexTiny) * count;
		v->vboSize = count;
		setVAOFormat();
	}
}

void chunkvertbufFree(struct chunk *c){
	if(c->vertbuf == NULL){return;}
	if(c->vertbuf->vbo){
		glDeleteBuffers(1,&c->vertbuf->vbo);
		allocatedGlBufferBytes -= sizeof(vertexTiny) * c->vertbuf->vboSize;
	}
	if(c->vertbuf->vao){glDeleteVertexArrays(1,&c->vertbuf->vao);}
	free(c->vertbuf);
	c->vertbuf = NULL;
}

#define FADE_IN_FRAMES 48

void chunkvertbufDrawOne(struct chunk *c, sideMask mask){
	struct chunkvertbuf *v = c->vertbuf;
	if(v->vao == 0 || v->count == 0){return;}
	uint bufOffset = 0;

	shaderTransform(sBlockMesh,c->x-subBlockViewOffset.x,c->y-subBlockViewOffset.y,c->z-subBlockViewOffset.z);

	glBindVertexArray(v->vao);
	if(mask == sideMaskALL || !glIsMultiDrawAvailable){
		glDrawArrays(GL_TRIANGLES,0,v->count);
		vboTrisCount += v->count / 3;
		drawCallCount++;
	}else{
		GLint first[sideMAX];
		GLsizei count[sideMAX];
		uint index = 0;
		bool reuseLastSide = false;
		for(side sideIndex = 0; sideIndex < sideMAX; sideIndex++){
			if(mask & (1 << sideIndex)){
				const uint cFirst = bufOffset + c->vertbuf->sideStart[sideIndex];
				const uint cCount = c->vertbuf->sideCount[sideIndex];
				if(cCount == 0){continue;}
				vboTrisCount += cCount / 3;
				if(reuseLastSide){
					count[index-1] += cCount;
				}else{
					first[index] = cFirst;
					count[index] = cCount;
					index++;
					reuseLastSide = true;
				}
			}else{
				reuseLastSide = false;
			}
		}
		glMultiDrawArrays(GL_TRIANGLES,first,count,index);
		drawCallCount++;
	}
}