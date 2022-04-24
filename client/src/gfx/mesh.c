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

#include "mesh.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gfx/shader.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gfx/gl.h"

vertex meshBuffer[(1<<14)-1];
mesh meshList[1024];
uint meshCount = 0;
mesh *meshFirstFree = NULL;

void meshAddVert(mesh *m, float x,float y,float z,float u,float v){
	meshBuffer[m->vertexCount] = (vertex){x,y,z,u,v,1.f};
	if(++m->vertexCount > countof(meshBuffer)){
		__asm__ volatile("int $0x03");
		fprintf(stderr,"meshBuffer Overflow!\n");
	}
}

void meshAddVertC(mesh *m, float x,float y,float z,float u,float v,float c){
	meshBuffer[m->vertexCount] = (vertex){x,y,z,u,v,c};
	if(++m->vertexCount > countof(meshBuffer)){
		__asm__ volatile("int $0x03");
		fprintf(stderr,"meshBuffer Overflow!\n");
	}
}

void meshEmpty(mesh *m){
	if(m == NULL){return;}
	m->vertexCount = 0;
}

static void meshCreateBuffers(mesh *m, const char *name){
	glGenVertexArrays(1, &m->vao);
	glBindVertexArray(m->vao);
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
	glEnableVertexAttribArray(SHADER_ATTRIDX_TEX);
	glEnableVertexAttribArray(SHADER_ATTRIDX_COLOR);

	glGenBuffers(1,&m->vbo);
	// TODO: do not generate an IBO name if not necessary
	glGenBuffers(1,&m->ibo);

	if(name != NULL){
		gfxObjectLabel(GL_VERTEX_ARRAY, m->vao, "%s VAO", name);
		gfxObjectLabel(GL_BUFFER, m->vbo, "%s VBO", name);
		gfxObjectLabel(GL_BUFFER, m->ibo, "%s IBO", name);
	}
}


static void meshDrawVBO(const mesh *m){
	glBindVertexArray(m->vao);
	glDrawArrays(GL_TRIANGLES, 0, m->vertexCount);
	vboTrisCount += m->vertexCount/3;
	drawCallCount++;
}

static void meshDrawIBO(const mesh *m){
	glBindVertexArray(m->vao);
	glDrawElements(GL_TRIANGLES, m->indexCount, m->indexType == meshIndexTypeU8 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, NULL);
	vboTrisCount += m->indexCount/3;
	drawCallCount++;
}

static void meshFinish(mesh *m, GLuint usage){
	if(m->vertexCount == 0){return;}
	glBindVertexArray(m->vao);
	glBindBuffer(GL_ARRAY_BUFFER,m->vbo);
	if(gfxUseSubData && (m->vboSize >= m->vertexCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, m->vertexCount*sizeof(vertex), meshBuffer);
	}else{
		glBufferData(GL_ARRAY_BUFFER, m->vertexCount*sizeof(vertex), meshBuffer, usage);
		glVertexAttribPointer(SHADER_ATTRIDX_POS,   3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(((char *)&meshBuffer[0].x) - ((char *)meshBuffer)));
		glVertexAttribPointer(SHADER_ATTRIDX_TEX,   2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(((char *)&meshBuffer[0].u) - ((char *)meshBuffer)));
		glVertexAttribPointer(SHADER_ATTRIDX_COLOR, 1, GL_FLOAT, GL_TRUE , sizeof(vertex), (void *)(((char *)&meshBuffer[0].c) - ((char *)meshBuffer)));
		m->vboSize = m->vertexCount;
	}
	if(m->indexCount > 0 && m->roIndexData != NULL){
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->indexCount*m->indexType, m->roIndexData, usage);
	}
	glBindVertexArray(0);
}

void meshFinishStatic(mesh *m){
	meshFinish(m,GL_STATIC_DRAW);
}
void meshFinishDynamic(mesh *m){
	meshFinish(m,GL_DYNAMIC_DRAW);
}

mesh *meshNew(const char *name){
	mesh *m = NULL;
	if(meshFirstFree != NULL){
		m = meshFirstFree;
		meshFirstFree = m->nextFree;
	}
	if(m == NULL){
		if(meshCount >= countof(meshList)){
			fprintf(stderr,"meshList Overflow!\n");
			return NULL;
		}
		m = &meshList[meshCount++];
	}
	if(name != NULL){lispDefineID("m-", name, m - meshList);}
	memset(m,0,sizeof(mesh));
	meshCreateBuffers(m, name);
	return m;
}

#define UNNORM(v,min,max,scale) (((float)v)/scale*(max-min) + min)
#define U8UNNORM(v,min,max) UNNORM(v,min,max,255)
#define U16UNNORM(v,min,max) UNNORM(v,min,max,65535)

mesh *meshNewAsset(const char *name, const assetMeshdata *asset){
	mesh *m = &meshList[meshCount++];
	memset(m,0,sizeof(mesh));
	meshCreateBuffers(m, name);
	m->vertexCount = asset->vertexCount;
	m->indexCount = asset->indexCount;
	m->roIndexData = asset->indexData.u8;
	m->indexType = asset->indexType;
	// Unpack the vertex data
	for(int i=0; i<asset->vertexCount; ++i){
		const assetVertex v = asset->vertexData[i];
		meshBuffer[i] = (vertex) {
			U8UNNORM(v.x, asset->bbox.min.x, asset->bbox.max.x),
			U8UNNORM(v.y, asset->bbox.min.y, asset->bbox.max.y),
			U8UNNORM(v.z, asset->bbox.min.z, asset->bbox.max.z),
			U8UNNORM(v.u, -1.25f, 1.25f),
			U8UNNORM(v.v, -1.25f, 1.25f),
			1.f
		};
	}
	if(name != NULL){lispDefineID("m-", name, m - meshList);}
	return m;
}

void meshFree(mesh *m){
	if(m == NULL){return;}
	glDeleteBuffers(1,&m->vbo);
	glDeleteBuffers(1,&m->ibo);
	m->nextFree = meshFirstFree;
	meshFirstFree = m;
}

void meshFreeAll(){
	for(uint i=0;i<meshCount;i++){
		meshFree(&meshList[i]);
	}
}

void meshDraw(const mesh *m){
	if(m->vertexCount == 0){return;}
	textureBind(m->tex);
	if(m->indexCount == 0){
		meshDrawVBO(m);
	}else{
		meshDrawIBO(m);
	}
}

mesh *meshGet(uint i){
	if(i > countof(meshList)){return NULL;}
	return &meshList[i];
}
uint meshIndex(const mesh *m){
	if(m == NULL){return 0;}
	return m - meshList;
}
