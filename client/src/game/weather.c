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

#pragma pack(push, 1)
typedef struct {
	float x,y,z;
	unsigned int color;
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
	const u8 cpart = cloudFrame++ & 31;

	shaderBind(sCloud);
	shaderSizeMul(sCloud,1.f + (player->aimFade * player->zoomFactor));
	for(int i=0;i<32;i++){
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
			glVertexAttribPointer(0, 3, GL_FLOAT        , GL_FALSE, sizeof(glCloud), (void *)(((char *)&cloudData[0].x) -     ((char *)cloudData)));
			glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glCloud), (void *)(((char *)&cloudData[0].color) - ((char *)cloudData)));
		}
		glDrawArrays(GL_POINTS,0,CLOUDS_MAX - parts[i].count);
	}
	parts[cloudFrame & 31].count = CLOUDS_MAX;
}

void cloudsDraw(int cx, int cy, int cz){
	if(cy&1){return;}
	const u8 cpart = (cx&3) | ((cy&0x4)) | ((cz&3)<<3 );
	if((cloudFrame&31) != cpart){return;}
	PROFILE_START();
	cloudChunk *part = &parts[cpart];
	part->base     = vecFloor(cloudOff);
	const ivec toff = ivecNewV(part->base);
	const int divx = (int)player->pos.x - (cx*256);
	const int divz = (int)player->pos.z - (cz*256);
	const int minx = MIN(divx,255);
	const int maxx = MAX(divx,  0);
	const int minz = MIN(divz,255);
	const int maxz = MAX(divz,  0);
	const ivec cp  = ivecNew(cx*256,cy*256,cz*256);
	const ivec pp  = ivecNewV(player->pos);
	ivec dp        = ivecZero();
	dp.y = (cp.y - pp.y)*(cp.y - pp.y);

	if(maxz<256){
		for(int x=maxx;x<256;x++){
			const int tx  = (x-toff.x)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=maxz;z<256;z++){
				const int tz  = (z-toff.z)&0xFF;
				const u8 v    = cloudTex[tx][tz];
				const int czz = cp.z+z;
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
		for(int x=minx;x>=0;x--){
			const int tx  = (x-toff.x)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=maxz;z<256;z++){
				const int tz  = (z-toff.z)&0xFF;
				const int czz = cp.z+z;
				const u8 v = cloudTex[tx][tz];
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
	}

	if(minz>0){
		for(int x=maxx;x<256;x++){
			const int tx = (x-toff.x)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=minz;z>=0;z--){
				const int tz  = (z-toff.z)&0xFF;
				const u8 v = cloudTex[tx][tz];
				const int czz = cp.z+z;
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
		for(int x=minx;x>=0;x--){
			const int tx = (x-toff.x)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=minz;z>=0;z--){
				const int tz  = (z-toff.z)&0xFF;
				const u8 v = cloudTex[tx][tz];
				const int czz = cp.z+z;
				if(v < cloudDensityMin){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
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
		glEnableVertexAttribArray (0);
		glEnableVertexAttribArray (2);
	}
	cloudsCalcColors();
}

void weatherDoRain(){}
