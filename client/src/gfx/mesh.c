#include "mesh.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gfx/shader.h"
#include "../tmp/assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gfx/gl.h"

vertex meshBuffer[16384];
mesh meshList[1024];
int  meshCount = 0;
mesh *meshFirstFree = NULL;

void meshAddVert(mesh *m, float x,float y,float z,float u,float v){
	meshBuffer[m->dataCount] = (vertex){x,y,z,u,v,1.f};
	if(++m->dataCount > (int)(sizeof(meshBuffer) / sizeof(vertex))){
		fprintf(stderr,"meshBuffer Overflow!\n");
	}
}

void meshAddVertC(mesh *m, float x,float y,float z,float u,float v,float c){
	meshBuffer[m->dataCount] = (vertex){x,y,z,u,v,c};
	if(++m->dataCount > (int)(sizeof(meshBuffer) / sizeof(vertex))){
		fprintf(stderr,"meshBuffer Overflow!\n");
	}
}

void meshEmpty(mesh *m){
	m->dataCount = 0;
}

void meshDrawVBO(const mesh *m){
	if(!m->vbo){return;}
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(((char *)&meshBuffer[0].x) - ((char *)meshBuffer)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(((char *)&meshBuffer[0].u) - ((char *)meshBuffer)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_TRUE , sizeof(vertex), (void *)(((char *)&meshBuffer[0].c) - ((char *)meshBuffer)));

	glDrawArrays(GL_TRIANGLES,0,m->dataCount);
	vboTrisCount += m->dataCount/3;
}

void meshFinish(mesh *m, unsigned int usage){
	if(!m->vbo){ glGenBuffers(1,&m->vbo); }
	glBindBuffer(GL_ARRAY_BUFFER,m->vbo);
	if(m->roData){
		glBufferData(GL_ARRAY_BUFFER, m->dataCount*sizeof(vertex),  m->roData, usage);
	}else{
		glBufferData(GL_ARRAY_BUFFER, m->dataCount*sizeof(vertex), meshBuffer, usage);
	}
}

mesh *meshNew(){
	mesh *m = NULL;
	if(meshFirstFree != NULL){
		m = meshFirstFree;
		meshFirstFree = m->nextFree;
	}
	if(m == NULL){
		if(meshCount >= (int)(sizeof(meshList) / sizeof(mesh))-1){
			fprintf(stderr,"meshList Overflow!\n");
			return NULL;
		}
		m = &meshList[meshCount++];
	}
	memset(m,0,sizeof(mesh));
	return m;
}

mesh *meshNewRO(const vertex *roData,size_t roSize){
	mesh *m = &meshList[meshCount++];
	memset(m,0,sizeof(mesh));
	m->dataCount = roSize;
	m->roData    = roData;
	return m;
}

void meshFree(mesh *m){
	if(m == NULL){return;}
	glDeleteBuffers(1,&m->vbo);
	m->nextFree = meshFirstFree;
	meshFirstFree = m;
}

void meshFreeAll(){
	for(int i=0;i<meshCount;i++){
		meshFree(&meshList[i]);
	}
}

void meshDraw(const mesh *m){
	textureBind(m->tex);
	meshDrawVBO(m);
}

void meshDrawLin(const mesh *m){
	textureBind(m->tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	meshDrawVBO(m);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}
