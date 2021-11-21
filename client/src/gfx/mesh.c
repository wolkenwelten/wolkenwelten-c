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

vertex meshBuffer[1<<16];
mesh meshList[1024];
uint meshCount = 0;
mesh *meshFirstFree = NULL;

void meshAddVert(mesh *m, float x,float y,float z,float u,float v){
	meshBuffer[m->dataCount] = (vertex){x,y,z,u,v,1.f};
	if(++m->dataCount > countof(meshBuffer)){
		fprintf(stderr,"meshBuffer Overflow!\n");
	}
}

void meshAddVertC(mesh *m, float x,float y,float z,float u,float v,float c){
	meshBuffer[m->dataCount] = (vertex){x,y,z,u,v,c};
	if(++m->dataCount > countof(meshBuffer)){
		fprintf(stderr,"meshBuffer Overflow!\n");
	}
}

void meshEmpty(mesh *m){
	if(m == NULL){return;}
	m->dataCount = 0;
}

static void meshCreateBuffers(mesh *m, const char *name){
	glGenVertexArrays(1, &m->vao);
	glBindVertexArray(m->vao);
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
	glEnableVertexAttribArray(SHADER_ATTRIDX_TEX);
	glEnableVertexAttribArray(SHADER_ATTRIDX_COLOR);

	glGenBuffers(1,&m->vbo);

	if(glIsDebugAvailable && name != NULL){
		char fullname[256];
		snprintf(fullname, sizeof(fullname), "%s VAO", name);
		glObjectLabel(GL_VERTEX_ARRAY, m->vao, -1, fullname);
		snprintf(fullname, sizeof(fullname), "%s VBO", name);
		glObjectLabel(GL_BUFFER, m->vbo, -1, fullname);
	}
}


static void meshDrawVBO(const mesh *m){
	if(m->dataCount == 0){return;}
	glBindVertexArray(m->vao);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glDrawArrays(GL_TRIANGLES,0,m->dataCount);
	vboTrisCount += m->dataCount/3;
	drawCallCount++;
}

static void meshFinish(mesh *m, uint usage){
	if(m->dataCount == 0){return;}
	glBindVertexArray(m->vao);
	glBindBuffer(GL_ARRAY_BUFFER,m->vbo);
	const void *data = m->roData == NULL ? meshBuffer : m->roData;
	if(gfxUseSubData && (m->vboSize >= m->dataCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, m->dataCount*sizeof(vertex),  data);
	}else{
		glBufferData(GL_ARRAY_BUFFER, m->dataCount*sizeof(vertex),  data, usage);
		glVertexAttribPointer(SHADER_ATTRIDX_POS,   3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(((char *)&meshBuffer[0].x) - ((char *)meshBuffer)));
		glVertexAttribPointer(SHADER_ATTRIDX_TEX,   2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(((char *)&meshBuffer[0].u) - ((char *)meshBuffer)));
		glVertexAttribPointer(SHADER_ATTRIDX_COLOR, 1, GL_FLOAT, GL_TRUE , sizeof(vertex), (void *)(((char *)&meshBuffer[0].c) - ((char *)meshBuffer)));
		m->vboSize = m->dataCount;
	}
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

mesh *meshNewRO(const char *name, const vertex *roData,size_t roSize){
	mesh *m = &meshList[meshCount++];
	memset(m,0,sizeof(mesh));
	meshCreateBuffers(m, name);
	m->dataCount = roSize;
	m->roData    = roData;
	if(name != NULL){lispDefineID("m-", name, m - meshList);}
	return m;
}

void meshFree(mesh *m){
	if(m == NULL){return;}
	glDeleteBuffers(1,&m->vbo);
	m->nextFree = meshFirstFree;
	meshFirstFree = m;
}

void meshFreeAll(){
	for(uint i=0;i<meshCount;i++){
		meshFree(&meshList[i]);
	}
}

void meshDraw(const mesh *m){
	if(m->dataCount == 0){return;}
	textureBind(m->tex);
	meshDrawVBO(m);
}

mesh *meshGet(uint i){
	if(i > countof(meshList)){return NULL;}
	return &meshList[i];
}
uint meshIndex(const mesh *m){
	if(m == NULL){return 0;}
	return m - meshList;
}
