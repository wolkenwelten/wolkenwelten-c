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
} glParticle;
#pragma pack(pop)

typedef struct particle {
	float vx,vy,vz,vsize;
} particle;

#define  PART_MAX (1<<16)
#define SPART_MAX (1<<14)

__attribute__((aligned(32))) glParticle glParticles   [PART_MAX];
__attribute__((aligned(32))) particle     particles   [PART_MAX];
__attribute__((aligned(32))) u32          particleRGBA[PART_MAX];
__attribute__((aligned(32))) uint         particleTTL [PART_MAX];
uint         particleCount = 0;
uint         particleVBO[2];

__attribute__((aligned(32))) glParticle glSparticles   [SPART_MAX];
__attribute__((aligned(32))) particle     sparticles   [SPART_MAX];
__attribute__((aligned(32))) u32          sparticleRGBA[SPART_MAX];
__attribute__((aligned(32))) uint         sparticleTTL [SPART_MAX];
uint         sparticleCount = 0;
uint         sparticleVBO[2];

void particleInit(){
	glGenBuffers(2,particleVBO);
	glGenBuffers(2,sparticleVBO);
}

void newParticleS(float x,float y,float z, u32 nrgba, float power, uint nttl){
	if((particleCount >= PART_MAX)){
		int i = rngValM(PART_MAX);
		particles[i]    = particles[--particleCount];
		glParticles[i]  = glParticles[particleCount];
		particleRGBA[i] = particleRGBA[particleCount];
		particleTTL[i]  = particleTTL[particleCount];
	}
	glParticles[particleCount]      = (glParticle){x,y,z,64.f};
	particles[particleCount].vx     = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vy     = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vz     = ((rngValf()-0.5f)/64.f)*power;
	particles[particleCount].vsize  = -0.8f;

	particleRGBA[particleCount]     = nrgba;
	particleTTL[particleCount]      = nttl;

	particleCount++;
}

void newSparticleV(vec pos, vec v, float size, float vsize, u32 rgba, uint ttl){
	if(sparticleCount >= SPART_MAX){
		int i = rngValM(SPART_MAX);
		sparticles[i]    = sparticles[--sparticleCount];
		glSparticles[i]  = glSparticles[sparticleCount];
		sparticleRGBA[i] = sparticleRGBA[sparticleCount];
		sparticleTTL[i]  = sparticleTTL[sparticleCount];
	}

	glSparticles [sparticleCount] = (glParticle){pos.x,pos.y,pos.z,size};
	sparticles   [sparticleCount] = (particle){v.x,v.y,v.z,vsize};
	sparticleRGBA[sparticleCount] = rgba;
	sparticleTTL [sparticleCount] = ttl;
	sparticleCount++;
}

void newParticleV(vec pos, vec v, float size, float vsize, u32 rgba,uint ttl){
	if(particleCount >= PART_MAX){
		int i = rngValM(PART_MAX);
		particles[i]   = particles[--particleCount];
		glParticles[i] = glParticles[particleCount];
		particleRGBA[i] = particleRGBA[particleCount];
		particleTTL[i]  = particleTTL[particleCount];
	}
	glParticles [particleCount] = (glParticle){pos.x,pos.y,pos.z,size};
	particles   [particleCount] = (particle){v.x,v.y,v.z,vsize};
	particleRGBA[particleCount] = rgba;
	particleTTL [particleCount] = ttl;
	particleCount++;
}

void newParticle(float x,float y,float z,float vx,float vy,float vz,float size,float vsize,u32 nrgba, uint nttl){
	if(particleCount >= PART_MAX){
		int i = rngValM(PART_MAX);
		particles[i]   = particles[--particleCount];
		glParticles[i] = glParticles[particleCount];
		particleRGBA[i] = particleRGBA[particleCount];
		particleTTL[i]  = particleTTL[particleCount];
	}
	glParticles  [particleCount] = (glParticle){ x, y, z, size};
	particles    [particleCount] = (particle){vx,vy,vz,vsize};
	particleRGBA [particleCount] = nrgba;
	particleTTL  [particleCount] = nttl;
	particleCount++;
}

#ifndef WW_ASM_PARTICLE_POS_UPDATE
void particlePosUpdate(){
	for(uint i=0;i<particleCount;i++){
		glParticles[i].x    += particles[i].vx;
		glParticles[i].y    += particles[i].vy;
		glParticles[i].z    += particles[i].vz;
		glParticles[i].size += particles[i].vsize;
	}
}
#endif

void sparticlePosUpdate(){
	const float spartv[8][4] = {
		{ 0.000001f,0.00004f, 0.000001f, 0.f},
		{ 0.000003f,0.00004f, 0.000003f, 0.f},
		{ 0.000002f,0.00004f,-0.000002f, 0.f},
		{ 0.000004f,0.00004f,-0.000004f, 0.f},
		{-0.000001f,0.00004f, 0.000001f, 0.f},
		{-0.000003f,0.00004f, 0.000003f, 0.f},
		{-0.000002f,0.00004f,-0.000002f, 0.f},
		{-0.000004f,0.00004f,-0.000004f, 0.f},
	};

	for(uint i=0;i<sparticleCount;i++){
		glSparticles[i].x    += sparticles[i].vx;
		glSparticles[i].y    += sparticles[i].vy;
		glSparticles[i].z    += sparticles[i].vz;
		glSparticles[i].size += sparticles[i].vsize;

		sparticles[i].vx     += spartv[i&0x7][0];
		sparticles[i].vy     += spartv[i&0x7][1];
		sparticles[i].vz     += spartv[i&0x7][2];
		sparticles[i].vsize  += spartv[i&0x7][3];
	}
}

void particleUpdate(){
	particlePosUpdate();
	sparticlePosUpdate();

	for(int i=particleCount-1;i>=0;i--){
		if(--particleTTL[i] <= 0){
			particles   [i] = particles [--particleCount];
			glParticles [i] = glParticles [particleCount];
			particleRGBA[i] = particleRGBA[particleCount];
			particleTTL [i] = particleTTL [particleCount];
			continue;
		}else if(particleTTL[i] < 128){
			particleRGBA[i] = (particleRGBA[i] & 0x00FFFFFF) | ((particleTTL[i] << 25) & 0xFF000000);
		}
	}

	for(int i=sparticleCount-1;i>=0;i--){
		if(--sparticleTTL[i] <= 0){
			sparticles   [i] = sparticles [--sparticleCount];
			glSparticles [i] = glSparticles [sparticleCount];
			sparticleRGBA[i] = sparticleRGBA[sparticleCount];
			sparticleTTL [i] = sparticleTTL [sparticleCount];
			continue;
		}else if(sparticleTTL[i] < 1024){
			sparticleRGBA[i] = (sparticleRGBA[i] & 0x00FFFFFF) | (sparticleTTL[i] << 22 & 0xFF000000);
		}
	}
}

void particleDraw(){
	if(!particleCount){return;}
	shaderBind(sParticle);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sCloud,1.f + (player->aimFade * player->zoomFactor));
	glDepthMask(GL_FALSE);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER,particleVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, particleCount*sizeof(glParticle), glParticles, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT        , GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER,particleVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, particleCount*sizeof(u32), particleRGBA, GL_STREAM_DRAW);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  0, (void *)0);
	glDrawArrays(GL_POINTS,0,particleCount);

	glBindBuffer(GL_ARRAY_BUFFER, sparticleVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sparticleCount*sizeof(glParticle), glSparticles, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT        , GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, sparticleVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sparticleCount*sizeof(u32), sparticleRGBA, GL_STREAM_DRAW);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  0, (void *)0);
	glDrawArrays(GL_POINTS,0,sparticleCount);

	glDepthMask(GL_TRUE);
	glEnableVertexAttribArray(1);
}
