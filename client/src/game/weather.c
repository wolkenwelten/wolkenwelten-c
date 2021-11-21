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

#include "weather.h"

#include "../main.h"
#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
#include "../gfx/particle.h"
#include "../game/character.h"
#include "../sdl/sdl.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/noise.h"
#include "../../../common/src/misc/profiling.h"

#include <math.h>
#include <stdio.h>

uint rainVAO;
uint rainVBO;
uint rainVBOSize = 0;

#ifdef __x86_64__
int rainFakeIters = 128;
#else
int rainFakeIters = 16;
#endif

#pragma pack(push, 1)
typedef struct {
	float x,y,z;
	u32 color;
} glCloud;
#pragma pack(pop)

#define CLOUDS_MAX (1<<18)

typedef struct {
	uint count,vbo,vao,vboSize;
	vec base;
} cloudChunk;

cloudChunk parts[32];
uint       cloudFrame = 0;
glCloud    cloudData[CLOUDS_MAX];

u32 cloudCT[128];
u32 cloudCB[128];

static inline void cloudPart(cloudChunk *part, float px,float py,float pz,float dd,u8 v){
	if(dd > cloudMaxD){return;}
	const float vf = v-cloudDensityMin;
	u32 a = v << 24;
	if(dd > cloudMinD){
		a = (u8)(v*(1.f-((dd - cloudMinD)/cloudFadeD))) << 24;
	}
	const float oy = 32.f;
	py += oy;
	const float vft = (vf/9.f);
	const float vfb = (vf/18.f);

	const u16 cx = px;
	const u16 cz = pz;
	const u16 cyb = py-vfb;
	const u16 cyt = ((int)(py+vfb))+1;
	for(u16 cy=cyb;cy < cyt;cy+=2){
		if(worldTryB(cx,cy,cz)){return;}
	}

	const   u32 ct = a | cloudCT[v-128];
	const   u32 cb = a | cloudCB[v-128];
	if(py < player->pos.y){
		cloudData[--part->count] = (glCloud){px,py+vft,pz,ct};
		cloudData[--part->count] = (glCloud){px,py-vfb,pz,cb};
	}else{
		cloudData[--part->count] = (glCloud){px,py-vfb,pz,cb};
		cloudData[--part->count] = (glCloud){px,py+vft,pz,ct};
	}
}

void cloudsRender(){
	gfxGroupStart("Clouds");
	const u8 cpart = cloudFrame++ & 31;
	cloudFrame &= 31;

	shaderBind(sCloud);
	shaderSizeMul(sCloud,player->zoomFactor);
	for(int i=0;i<32;i++){
		if((CLOUDS_MAX - parts[i].count) == 0){ continue; }
		matMov(matMVP,matView);
		const vec transOff = vecSub(cloudOff,parts[i].base);
		matMulTrans(matMVP,transOff.x,transOff.y,transOff.z);
		matMul(matMVP,matMVP,matProjection);
		shaderMatrix(sCloud,matMVP);

		glBindVertexArray(parts[i].vao);
		if(i == cpart){
			glBindBuffer(GL_ARRAY_BUFFER, parts[cpart].vbo);
			if(gfxUseSubData && (parts[cpart].vboSize >= (CLOUDS_MAX - parts[cpart].count))){
				glBufferSubData(GL_ARRAY_BUFFER, 0, (CLOUDS_MAX - parts[cpart].count)*sizeof(glCloud), &cloudData[parts[cpart].count]);
			}else{
				glBufferData(GL_ARRAY_BUFFER, (CLOUDS_MAX - parts[cpart].count)*sizeof(glCloud), &cloudData[parts[cpart].count], GL_DYNAMIC_DRAW);
				parts[cpart].vboSize = (CLOUDS_MAX - parts[cpart].count);
			}
			glVertexAttribPointer(SHADER_ATTRIDX_POS,   3, GL_FLOAT        , GL_FALSE, sizeof(glCloud), (void *)(((char *)&cloudData[0].x) -     ((char *)cloudData)));
			glVertexAttribPointer(SHADER_ATTRIDX_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glCloud), (void *)(((char *)&cloudData[0].color) - ((char *)cloudData)));
		}
		glDrawArrays(GL_POINTS,0,CLOUDS_MAX - parts[i].count);
	}
	parts[cloudFrame & 31].count = CLOUDS_MAX;
	gfxGroupEnd();
}

void cloudsDraw(int cx, int cy, int cz){
	if(cy&1){return;}
	const u8 cpart = (cx&3) | ((cy&0x4)) | ((cz&3)<<3 );
	if((cloudFrame&31) != cpart){return;}
	PROFILE_START();
	cloudChunk *part = &parts[cpart];
	part->base     = vecFloor(cloudOff);
	//const ivec toff = ivecNewV(part->base);
	const int toffx = part->base.x;
	const int toffz = part->base.z;
	const int divx = (int)player->pos.x - (cx*256);
	const int divz = (int)player->pos.z - (cz*256);
	const int minx = MIN(divx,255);
	const int maxx = MAX(divx,  0);
	const int minz = MIN(divz,255);
	const int maxz = MAX(divz,  0);
	const int cpx  = cx<<8;
	const int cpy  = cy<<8;
	const int cpz  = cz<<8;
	vec dp         = vecZero();
	dp.y = ((cpy) - player->pos.y);
	dp.y *= dp.y;

	if(maxz<256){
		for(int x=maxx;x<256;x++){
			const int tx  = (x-toffx)&0xFF;
			const int cxx = cpx+x;
			dp.x = (cxx - player->pos.x)*(cxx - player->pos.x);
			for(int z=maxz;z<256;z++){
				const int tz  = (z-toffz)&0xFF;
				const u8 v    = cloudTex[tx][tz];
				const int czz = cpz+z;
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - player->pos.z)*(czz - player->pos.z);
				cloudPart(part,cxx,cpy,czz,vecSum(dp),v);
			}
		}
		for(int x=minx;x>=0;x--){
			const int tx  = (x-toffx)&0xFF;
			const int cxx = cpx+x;
			dp.x = (cxx - player->pos.x)*(cxx - player->pos.x);
			for(int z=maxz;z<256;z++){
				const int tz  = (z-toffz)&0xFF;
				const int czz = cpz+z;
				const u8 v = cloudTex[tx][tz];
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - player->pos.z)*(czz - player->pos.z);
				cloudPart(part,cxx,cpy,czz,vecSum(dp),v);
			}
		}
	}

	if(minz>0){
		for(int x=maxx;x<256;x++){
			const int tx = (x-toffx)&0xFF;
			const int cxx = cpx+x;
			dp.x = (cxx - player->pos.x)*(cxx - player->pos.x);
			for(int z=minz;z>=0;z--){
				const int tz  = (z-toffz)&0xFF;
				const u8 v = cloudTex[tx][tz];
				const int czz = cpz+z;
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - player->pos.z)*(czz - player->pos.z);
				cloudPart(part,cxx,cpy,czz,vecSum(dp),v);
			}
		}
		for(int x=minx;x>=0;x--){
			const int tx = (x-toffx)&0xFF;
			const int cxx = cpx+x;
			dp.x = (cxx - player->pos.x)*(cxx - player->pos.x);
			for(int z=minz;z>=0;z--){
				const int tz  = (z-toffz)&0xFF;
				const u8 v = cloudTex[tx][tz];
				const int czz = cpz+z;
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - player->pos.z)*(czz - player->pos.z);
				cloudPart(part,cxx,cpy,czz,vecSum(dp),v);
			}
		}
	}
	PROFILE_STOP();
}

void cloudsCalcColors(){
	static float lastBrightness = 100.f;
	static u8 lastCloudDensityMin = 170;
	if((fabsf(lastBrightness - skyBrightness) < 0.01f) && (cloudDensityMin == lastCloudDensityMin)){return;}
	lastBrightness = skyBrightness;
	lastCloudDensityMin = cloudDensityMin;
	for(int i=0;i<128;i++){
		const u8 cdm = cloudDensityMin;
		const u32 v  = i+128;
		const u32 ta = MIN(255,((cdm+30)+((256 - v)/2))) * skyBrightness;
		const u32 tb = MIN(255,((cdm+ 8)+((256 - v)/4))) * skyBrightness;
		const u32 ba = ((cdm- 8)+((256 - v)  ))          * skyBrightness;
		const u32 bb = ((cdm-24)+((256 - v)/2))          * skyBrightness;

		cloudCT[i] = ((tb<<16) | (ta<<8) | ta) & 0x00FFFFFF;
		cloudCB[i] = ((bb<<16) | (ba<<8) | ba) & 0x00FFFFFF;
	}
}

void cloudsInitGfx(){
	for(int i=0;i<32;i++){
		parts[i].count = CLOUDS_MAX;
		parts[i].base  = vecZero();
		glGenVertexArrays(1,&parts[i].vao);
		glBindVertexArray(parts[i].vao);
		glGenBuffers(1,&parts[i].vbo);
		glBindBuffer(GL_ARRAY_BUFFER, parts[i].vbo);
		glEnableVertexAttribArray (SHADER_ATTRIDX_POS);
		glEnableVertexAttribArray (SHADER_ATTRIDX_COLOR);

		if(glIsDebugAvailable){
			char name[16];
			snprintf(name, sizeof(name), "Clouds VAO #%d", i);
			glObjectLabel(GL_VERTEX_ARRAY, parts[i].vao, -1, name);
			snprintf(name, sizeof(name), "Clouds VAO #%d", i);
			glObjectLabel(GL_BUFFER, parts[i].vbo, -1, name);
		}
	}
	cloudsCalcColors();

	glGenVertexArrays(1, &rainVAO);
	glGenBuffers     (1, &rainVBO);
	glBindVertexArray    (rainVAO);
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
}

void rainFakeDrops(){
	if(rainIntensity == 0){return;}
	vec pos = player->pos;
	int cy = (((int)pos.y) & 0xFE00)-0x100;
	pos.y = cy + (32.f - 256.f);
	for(uint i=0;i<8;i++){
		float v = 48.f;
		for(int ii=0;ii<4;ii++){
			for(int iii=0;iii<rainFakeIters;iii++){
				if(rngValA(255) > rainIntensity){continue;}
				const vec rpos = vecAdd(pos,vecMul(vecRng(), vecNew( v,0.f, v)));
				const u8 vv = cloudTex[(uint)(rpos.x - cloudOff.x)&0xFF][(uint)(rpos.z - cloudOff.z)&0xFF];
				if(vv < cloudDensityMin){continue;}
				rainNew(rpos);
			}
			v *= 2.f;
		}
		pos.y += 256.f;
	}
}

void rainDrawAll(){
	gfxGroupStart("Rain");
	if(!rainCount){return;}
	rainFakeDrops();

	shaderBind(sRain);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sRain,player->zoomFactor);
	glDepthMask(GL_FALSE);

	glBindVertexArray(rainVAO);
	glBindBuffer(GL_ARRAY_BUFFER,rainVBO);
	if(gfxUseSubData && (rainVBOSize >= rainCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, rainCount*sizeof(glRainDrop), glRainDrops);
	}else{
		glBufferData(GL_ARRAY_BUFFER, rainCount*sizeof(glRainDrop), glRainDrops, GL_DYNAMIC_DRAW);
		rainVBOSize = rainCount;
	}
	glVertexAttribPointer(SHADER_ATTRIDX_POS, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_POINTS,0,rainCount);

	glDepthMask(GL_TRUE);
	gfxGroupEnd();
}

void rainRecvUpdate(const packet *p){
	const vec pos  = vecNew(p->v.f[0],p->v.f[1],p->v.f[2]);
	const vec dist = vecSub(pos,player->pos);
	const float dd = vecDot(dist,dist);
	if(dd > renderDistanceSquare){return;}
	rainNew(pos);
}

void weatherDoRain(){}
