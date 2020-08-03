#include "particle.h"
#include "../tmp/assets.h"
#include "../../../common/src/misc/misc.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"

#include <stdio.h>
#include <stdlib.h>
#include "../gfx/glew.h"

#pragma pack(push, 1)
typedef struct glParticle {
	float x,y,z;
	unsigned int rgba;
} glParticle;
#pragma pack(pop)

typedef struct particle {
	float vx,vy,vz,vvx,vvy,vvz;
	unsigned int rgba;
	int ttl;
} particle;

unsigned int particleVBO = 0;

glParticle glParticles[1<<18];
int glParticleCount = 0;

particle particles[1<<18];
int particleCount = 0;

void particleInit(){
	glGenBuffers(1,&particleVBO);
}

void newParticleS(float x,float y,float z, unsigned int nrgba,float power,int nttl){
	if((glParticleCount >= (1<<18)) || (particleCount >= (1<<18))){
		return;
	}
	glParticles[glParticleCount].x    = x;
	glParticles[glParticleCount].y    = y;
	glParticles[glParticleCount].z    = z;
	glParticles[glParticleCount].rgba = nrgba;
	glParticleCount++;

	particles[particleCount].vx   = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vy   = (0.02f)*power;
	particles[particleCount].vz   = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vvx  = 0.f;
	particles[particleCount].vvy  = -0.0005f;
	particles[particleCount].vvz  = 0.f;
	particles[particleCount].rgba = nrgba;
	particles[particleCount].ttl  = nttl;
	particleCount++;
}

void newParticle(float x,float y,float z,float vx,float vy,float vz,float vvx,float vvy,float vvz,unsigned int nrgba,int nttl){
	if((glParticleCount >= (1<<18)) || (particleCount >= (1<<18))){
		return;
	}
	glParticles[glParticleCount].x    = x;
	glParticles[glParticleCount].y    = y;
	glParticles[glParticleCount].z    = z;
	glParticles[glParticleCount].rgba = nrgba;
	glParticleCount++;

	particles[particleCount].vx   = vx;
	particles[particleCount].vy   = vy;
	particles[particleCount].vz   = vz;
	particles[particleCount].vvx  = vvx;
	particles[particleCount].vvy  = vvy;
	particles[particleCount].vvz  = vvz;
	particles[particleCount].rgba = nrgba;
	particles[particleCount].ttl  = nttl;
	particleCount++;
}

void particleUpdate(){
	for(int i=particleCount-1;i>=0;i--){
		if(--particles[i].ttl <= 0){
			particles[i] = particles[--particleCount];
			glParticles[i] = glParticles[--glParticleCount];
			continue;
		}
		if(particles[i].ttl < 32){
			glParticles[i].rgba = (particles[i].rgba & 0x00FFFFFF) | (particles[i].ttl << 27);
		}
		glParticles[i].x += particles[i].vx;
		glParticles[i].y += particles[i].vy;
		glParticles[i].z += particles[i].vz;
		particles[i].vx  += particles[i].vvx;
		particles[i].vy  += particles[i].vvy;
		particles[i].vz  += particles[i].vvz;
	}
}

void particleDraw(){
	float matMVP[16];
	if(!glParticleCount){return;}
	shaderBind(sParticle);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);


	glBindBuffer(GL_ARRAY_BUFFER,particleVBO);
	glBufferData(GL_ARRAY_BUFFER, glParticleCount*sizeof(glParticle), glParticles, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT        , GL_FALSE, sizeof(glParticle), (void *)(((char *)&glParticles[0].x) - ((char *)glParticles)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glParticle), (void *)(((char *)&glParticles[0].rgba)  - ((char *)glParticles)));
	glDrawArrays(GL_POINTS,0,glParticleCount);
}
