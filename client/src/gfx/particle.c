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

#include "particle.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../../../common/src/asm/asm.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <stdio.h>
#include <stdlib.h>
#include "../gfx/gl.h"

#pragma pack(push, 1)
typedef struct {
	float x,y,z,size;
} glParticle;
#pragma pack(pop)

typedef struct {
	float vx,vy,vz,vsize;
} particle;

#define  PART_MAX (1<<16)
#define SPART_MAX (1<<14)

__attribute__((aligned(32))) glParticle glParticles   [PART_MAX+4];
__attribute__((aligned(32))) particle     particles   [PART_MAX+4];
__attribute__((aligned(32))) u32          particleRGBA[PART_MAX+4];
__attribute__((aligned(32))) uint         particleTTL [PART_MAX+4];
uint         particleCount = 0;
uint         particleVBO[2];
uint         particleVBOSize = 0;
uint         particleVAO;

__attribute__((aligned(32))) glParticle glSparticles   [SPART_MAX+4];
__attribute__((aligned(32))) particle     sparticles   [SPART_MAX+4];
__attribute__((aligned(32))) u32          sparticleRGBA[SPART_MAX+4];
__attribute__((aligned(32))) uint         sparticleTTL [SPART_MAX+4];
uint         sparticleCount;
uint         sparticleVBO[2];
uint         sparticleVBOSize;
uint         sparticleVAO;

void particleInit(){
	glGenBuffers(2,particleVBO);
	glGenBuffers(2,sparticleVBO);
	particleVBOSize = 0;
	sparticleVBOSize = 0;

	glGenVertexArrays(1, &particleVAO);
	if(glIsDebugAvailable){glObjectLabel(GL_VERTEX_ARRAY,particleVAO,-1,"particle VAO");}
	glBindVertexArray(particleVAO);
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
	glEnableVertexAttribArray(SHADER_ATTRIDX_COLOR);

	glGenVertexArrays(1,&sparticleVAO);
	if(glIsDebugAvailable){glObjectLabel(GL_VERTEX_ARRAY,particleVAO,-1,"sparticle VAO");}
	glBindVertexArray(sparticleVAO);
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
	glEnableVertexAttribArray(SHADER_ATTRIDX_COLOR);
}

void newParticleS(float x,float y,float z, u32 nrgba, float power, uint nttl){
	if((particleCount >= PART_MAX)){
		int i = rngValM(PART_MAX);
		particleCount = MIN(PART_MAX-1,particleCount);
		particles[i]    = particles[particleCount];
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
		sparticleCount = MIN(SPART_MAX-1,sparticleCount);
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
		particleCount = MIN(PART_MAX-1,particleCount);
		particles[i]    = particles[--particleCount];
		glParticles[i]  = glParticles[particleCount];
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
		particleCount = MIN(PART_MAX-1,particleCount);
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

void particlePosUpdatePortable(){
	for(uint i=0;i<particleCount;i++){
		glParticles[i].x    += particles[i].vx;
		glParticles[i].y    += particles[i].vy;
		glParticles[i].z    += particles[i].vz;
		glParticles[i].size += particles[i].vsize;
	}
}

__attribute__((aligned(32))) const float sparticleVV[4][4] = {
	{  0.000001f,0.00004f, 0.000004f, 0.f},
	{ -0.000004f,0.00004f, 0.000001f, 0.f},
	{ -0.000001f,0.00004f,-0.000004f, 0.f},
	{  0.000004f,0.00004f,-0.000001f, 0.f},
};

void sparticlePosUpdatePortable(){
	for(uint i=0;i<sparticleCount;i++){
		glSparticles[i].x    += sparticles[i].vx;
		glSparticles[i].y    += sparticles[i].vy;
		glSparticles[i].z    += sparticles[i].vz;
		glSparticles[i].size += sparticles[i].vsize;

		sparticles[i].vx     += sparticleVV[i&0x3][0];
		sparticles[i].vy     += sparticleVV[i&0x3][1];
		sparticles[i].vz     += sparticleVV[i&0x3][2];
		sparticles[i].vsize  += sparticleVV[i&0x3][3];
	}
}

void particleUpdate(){
	PROFILE_START();

	particlePosUpdate();
	sparticlePosUpdate();

	for(int i=particleCount-1;i>=0;i--){
		if(--particleTTL[i] <= 0){
			particles   [i] = particles [--particleCount];
			glParticles [i] = glParticles [particleCount];
			particleRGBA[i] = particleRGBA[particleCount];
			particleTTL [i] = particleTTL [particleCount];
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
		}else if((sparticleRGBA[i] & 0xFF000000) != 0xFF000000){
			if((sparticleTTL[i] & 0x3) == 0){
				sparticleRGBA[i] += 0x01000000;
			}
		}
	}

	PROFILE_STOP();
}

void particleDraw(){
	gfxGroupStart("Particles");
	if(!particleCount && !sparticleCount){return;}
	shaderBind(sParticle);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sCloud,player->zoomFactor);
	glDepthMask(GL_FALSE);

	glBindVertexArray(particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER,particleVBO[0]);
	if(gfxUseSubData && (particleVBOSize >+ particleCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount*sizeof(glParticle), glParticles);
	}else{
		glBufferData(GL_ARRAY_BUFFER, particleCount*sizeof(glParticle), glParticles, GL_DYNAMIC_DRAW);
	}
	glVertexAttribPointer(SHADER_ATTRIDX_POS, 4, GL_FLOAT        , GL_FALSE, 0, (void *)0);

	glBindBuffer(GL_ARRAY_BUFFER,particleVBO[1]);
	if(gfxUseSubData && (particleVBOSize >+ particleCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount*sizeof(u32), particleRGBA);
	}else{
		glBufferData(GL_ARRAY_BUFFER, particleCount*sizeof(u32), particleRGBA, GL_DYNAMIC_DRAW);
		particleVBOSize = particleCount;
	}
	glVertexAttribPointer(SHADER_ATTRIDX_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE,  0, (void *)0);
	glDrawArrays(GL_POINTS,0,particleCount);

	glBindVertexArray(sparticleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sparticleVBO[0]);
	if(gfxUseSubData && (sparticleVBOSize >= sparticleCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, sparticleCount*sizeof(glParticle), glSparticles);
	}else{
		glBufferData(GL_ARRAY_BUFFER, sparticleCount*sizeof(glParticle), glSparticles, GL_DYNAMIC_DRAW);
	}
	glVertexAttribPointer(SHADER_ATTRIDX_POS, 4, GL_FLOAT        , GL_FALSE, 0, (void *)0);

	glBindBuffer(GL_ARRAY_BUFFER, sparticleVBO[1]);
	if(gfxUseSubData && (sparticleVBOSize >= sparticleCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, sparticleCount*sizeof(u32), sparticleRGBA);
	}else{
		glBufferData(GL_ARRAY_BUFFER, sparticleCount*sizeof(u32), sparticleRGBA, GL_DYNAMIC_DRAW);
		sparticleVBOSize = sparticleCount;
	}
	glVertexAttribPointer(SHADER_ATTRIDX_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE,  0, (void *)0);
	glDrawArrays(GL_POINTS,0,sparticleCount);


	glDepthMask(GL_TRUE);
	gfxGroupEnd();
}
