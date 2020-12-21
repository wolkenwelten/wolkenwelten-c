#include "particle.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../tmp/assets.h"
#include "../../../common/src/misc/misc.h"

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

#define PART_MAX (1<<18)

glParticle glParticles[PART_MAX];
particle particles[PART_MAX];
int particleCount = 0;

void particleInit(){
	glGenBuffers(1,&particleVBO);
}

void newParticleS(float x,float y,float z, unsigned int nrgba,float power,int nttl){
	if((particleCount >= PART_MAX)){
		int i = rngValM(PART_MAX);
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

void newParticleV(vec pos, vec v, vec vv,float size, float vsize,uint rgba,uint ttl){
	if(particleCount >= PART_MAX){
		int i = rngValM(PART_MAX);
		particles[i]   = particles[--particleCount];
		glParticles[i] = glParticles[particleCount];
	}
	glParticles[particleCount].x    = pos.x;
	glParticles[particleCount].y    = pos.y;
	glParticles[particleCount].z    = pos.z;
	glParticles[particleCount].size = size;
	glParticles[particleCount].rgba = rgba;
	particles[particleCount].vx     = v.x;
	particles[particleCount].vy     = v.y;
	particles[particleCount].vz     = v.z;
	particles[particleCount].vvx    = vv.x;
	particles[particleCount].vvy    = vv.y;
	particles[particleCount].vvz    = vv.z;
	particles[particleCount].vsize  = vsize;
	particles[particleCount].rgba   = rgba;
	particles[particleCount].ttl    = ttl;
	particleCount++;
}

void newParticle(float x,float y,float z,float vx,float vy,float vz,float vvx,float vvy,float vvz,float size,float vsize,unsigned int nrgba,int nttl){
	if(particleCount >= PART_MAX){
		int i = rngValM(PART_MAX);
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
		if(particles[i].ttl < 128){
			glParticles[i].rgba = (particles[i].rgba & 0x00FFFFFF) | (particles[i].ttl << 25);
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
	if(!particleCount){return;}
	shaderBind(sParticle);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sCloud,1.f + (player->aimFade * player->zoomFactor));

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
