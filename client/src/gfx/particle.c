#include "particle.h"
#include "../tmp/assets.h"
#include "../../../common/src/misc/misc.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"

#include <stdio.h>
#include <stdlib.h>
#include "../gfx/gl.h"

#pragma pack(push, 1)
typedef struct glParticle {
	float x,y,z,size;
	unsigned int rgba;
} glParticle;
#pragma pack(pop)

typedef struct particle {
	float vx,vy,vz,vvx,vvy,vvz,vsize;
	unsigned int rgba;
	int ttl;
} particle;

unsigned int particleVBO = 0;

glParticle glParticles[1<<19];
particle particles[1<<19];
int particleCount = 0;

void particleInit(){
	glGenBuffers(1,&particleVBO);
}

void newParticleS(float x,float y,float z, unsigned int nrgba,float power,int nttl){
	if((particleCount >= (1<<19))){
		int i = rngValM(1<<19);
		particles[i]   = particles[--particleCount];
		glParticles[i] = glParticles[particleCount];
	}
	glParticles[particleCount].x    = x;
	glParticles[particleCount].y    = y;
	glParticles[particleCount].z    = z;
	glParticles[particleCount].size = 64.f;
	glParticles[particleCount].rgba = nrgba;
	particles[particleCount].vx     = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vy     = (0.02f)*power;
	particles[particleCount].vz     = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vvx    = 0.f;
	particles[particleCount].vvy    = -0.0005f;
	particles[particleCount].vvz    = 0.f;
	particles[particleCount].vsize  = 0.1f;
	particles[particleCount].rgba   = nrgba;
	particles[particleCount].ttl    = nttl;
	particleCount++;
}

void newParticle(float x,float y,float z,float vx,float vy,float vz,float vvx,float vvy,float vvz,float size,float vsize,unsigned int nrgba,int nttl){
	if(particleCount >= (1<<19)){
		int i = rngValM(1<<19);
		particles[i]   = particles[--particleCount];
		glParticles[i] = glParticles[particleCount];
	}
	glParticles[particleCount].x    = x;
	glParticles[particleCount].y    = y;
	glParticles[particleCount].z    = z;
	glParticles[particleCount].size = size;
	glParticles[particleCount].rgba = nrgba;
	particles[particleCount].vx     = vx;
	particles[particleCount].vy     = vy;
	particles[particleCount].vz     = vz;
	particles[particleCount].vvx    = vvx;
	particles[particleCount].vvy    = vvy;
	particles[particleCount].vvz    = vvz;
	particles[particleCount].vsize  = vsize;
	particles[particleCount].rgba   = nrgba;
	particles[particleCount].ttl    = nttl;
	particleCount++;
}

void particleUpdate(){
	for(int i=particleCount-1;i>=0;i--){
		if(--particles[i].ttl <= 0){
			particles[i] = particles[--particleCount];
			glParticles[i] = glParticles[particleCount];
			continue;
		}
		if(particles[i].ttl < 32){
			glParticles[i].rgba = (particles[i].rgba & 0x00FFFFFF) | (particles[i].ttl << 27);
		}
		glParticles[i].x    += particles[i].vx;
		glParticles[i].y    += particles[i].vy;
		glParticles[i].z    += particles[i].vz;
		glParticles[i].size += particles[i].vsize;
		particles[i].vx     += particles[i].vvx;
		particles[i].vy     += particles[i].vvy;
		particles[i].vz     += particles[i].vvz;
	}
}

void particleDraw(){
	float matMVP[16];
	if(!particleCount){return;}
	shaderBind(sParticle);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);


	glBindBuffer(GL_ARRAY_BUFFER,particleVBO);
	glBufferData(GL_ARRAY_BUFFER, particleCount*sizeof(glParticle), glParticles, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 3, GL_FLOAT        , GL_FALSE, sizeof(glParticle), (void *)(((char *)&glParticles[0].x) - ((char *)glParticles)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glParticle), (void *)(((char *)&glParticles[0].rgba)  - ((char *)glParticles)));
	glVertexAttribPointer(3, 1, GL_FLOAT        , GL_FALSE, sizeof(glParticle), (void *)(((char *)&glParticles[0].size) - ((char *)glParticles)));
	glDrawArrays(GL_POINTS,0,particleCount);

	glDisableVertexAttribArray(3);
}
