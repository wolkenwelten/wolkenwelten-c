#include "particle.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../tmp/assets.h"
#include "../../../common/src/asm/asm.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <stdlib.h>
#include "../gfx/gl.h"

#pragma pack(push, 1)
typedef struct glParticle {
	float x,y,z,size;
	u32 rgba;
} glParticle;
#pragma pack(pop)

typedef struct particle {
	float vx,vy,vz,vsize;
	float vvx,vvy,vvz;
	u32 rgba;
	int ttl;
} particle;

#define  PART_MAX (1<<17)
#define SPART_MAX (1<<16)

glParticle glParticles[PART_MAX];
particle     particles[PART_MAX];
uint         particleCount = 0;
uint         particleVBO   = 0;

glParticle glSparticles[SPART_MAX];
particle     sparticles[SPART_MAX];
uint         sparticleCount = 0;
uint         sparticleVBO   = 0;

void particleInit(){
	glGenBuffers(1,&particleVBO);
	glGenBuffers(1,&sparticleVBO);
}

void newParticleS(float x,float y,float z, u32 nrgba, float power,int nttl){
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

void newSparticleV(vec pos, vec v, vec vv,float size, float vsize,uint rgba,uint ttl){
	if(sparticleCount >= SPART_MAX){
		int i = rngValM(SPART_MAX);
		sparticles[i]   = sparticles[--sparticleCount];
		glSparticles[i] = glSparticles[sparticleCount];
	}
	glParticle *glp = &glSparticles[sparticleCount];
	  particle *p   =   &sparticles[sparticleCount];
	sparticleCount++;

	glp->x    = pos.x;
	glp->y    = pos.y;
	glp->z    = pos.z;
	glp->size = size;
	glp->rgba = rgba;
	p->vx     = v.x;
	p->vy     = v.y;
	p->vz     = v.z;
	p->vvx    = vv.x;
	p->vvy    = vv.y;
	p->vvz    = vv.z;
	p->vsize  = vsize;
	p->rgba   = rgba;
	p->ttl    = ttl;
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


#ifndef WW_ASM_PARTICLE_UPDATE
void particleUpdate(){
	for(int i=particleCount-1;i>=0;i--){
		if(--particles[i].ttl <= 0){
			particles[i] = particles[--particleCount];
			glParticles[i] = glParticles[particleCount];
			continue;
		}else if(particles[i].ttl < 128){
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

	for(int i=sparticleCount-1;i>=0;i--){
		if(--sparticles[i].ttl <= 0){
			sparticles[i] = sparticles[--sparticleCount];
			glSparticles[i] = glSparticles[sparticleCount];
			continue;
		}
		if(sparticles[i].ttl < 1024){
			glSparticles[i].rgba = (sparticles[i].rgba & 0x00FFFFFF) | (sparticles[i].ttl << 22 & 0xFF000000);
		}
		glSparticles[i].x    += sparticles[i].vx;
		glSparticles[i].y    += sparticles[i].vy;
		glSparticles[i].z    += sparticles[i].vz;
		glSparticles[i].size += sparticles[i].vsize;
		sparticles[i].vx     += sparticles[i].vvx;
		sparticles[i].vy     += sparticles[i].vvy;
		sparticles[i].vz     += sparticles[i].vvz;
	}
}
#endif

void particleDraw(){
	if(!particleCount){return;}
	shaderBind(sParticle);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sCloud,1.f + (player->aimFade * player->zoomFactor));
	glDepthMask(GL_FALSE);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER,particleVBO);
	glBufferData(GL_ARRAY_BUFFER, particleCount*sizeof(glParticle), glParticles, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT        , GL_FALSE, sizeof(glParticle), (void *)(((char *)&glParticles[0].x)    - ((char *)glParticles)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glParticle), (void *)(((char *)&glParticles[0].rgba) - ((char *)glParticles)));
	glDrawArrays(GL_POINTS,0,particleCount);

	glBindBuffer(GL_ARRAY_BUFFER, sparticleVBO);
	glBufferData(GL_ARRAY_BUFFER, sparticleCount*sizeof(glParticle), glSparticles, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT        , GL_FALSE, sizeof(glParticle), (void *)(((char *)&glParticles[0].x)    - ((char *)glParticles)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glParticle), (void *)(((char *)&glParticles[0].rgba) - ((char *)glParticles)));
	glDrawArrays(GL_POINTS,0,sparticleCount);

	glDepthMask(GL_TRUE);
	glEnableVertexAttribArray(1);
}
