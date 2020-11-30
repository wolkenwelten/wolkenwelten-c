#include "clouds.h"

#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
#include "../gfx/particle.h"
#include "../game/character.h"
#include "../sdl/sdl.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/noise.h"

#include <math.h>

#pragma pack(push, 1)
typedef struct {
	float x,y,z;
	unsigned int color;
} glCloud;
#pragma pack(pop)

#define CLOUD_FADED (65536)
#define CLOUD_MIND  (65536*3)
#define CLOUD_DENSITY_MIN 170
#define CLOUDS_MAX (1<<19)

typedef struct {
	uint count,vbo;
	float base;
} cloudChunk;

cloudChunk parts[8];
u8         cloudTex[256][256];
float      cloudOffset = 0.f;
uint       cloudFrame = 0;
glCloud    cloudData[CLOUDS_MAX];

u32 cloudCT[128];
u32 cloudCB[128];

static inline void cloudPart(cloudChunk *part, float px,float py,float pz,float dd,u8 v){
	if(dd > (CLOUD_MIND+CLOUD_FADED)){return;}
	const float vf = v-CLOUD_DENSITY_MIN;
	u32 a = v << 24;
	if(dd > CLOUD_MIND){
		a = (u8)(v*(1.f-((dd - CLOUD_MIND)/CLOUD_FADED))) << 24;
	}
	const   u32 ct = a | cloudCT[v-128];
	const   u32 cb = a | cloudCB[v-128];
	const float oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED)) * 23.f + 19.f;
	py += oy;

	if(py < player->pos.y){
		cloudData[--part->count] = (glCloud){px,py+vf/ 6.f,pz,ct};
		cloudData[--part->count] = (glCloud){px,py-vf/12.f,pz,cb};
	}else{
		cloudData[--part->count] = (glCloud){px,py-vf/12.f,pz,cb};
		cloudData[--part->count] = (glCloud){px,py+vf/ 6.f,pz,ct};
	}
}

void cloudsRender(){
	const u8 cpart = cloudFrame++ & 7;
	glBindBuffer(GL_ARRAY_BUFFER, parts[cpart].vbo);
	glBufferData(GL_ARRAY_BUFFER, (CLOUDS_MAX - parts[cpart].count)*sizeof(glCloud), &cloudData[parts[cpart].count], GL_STREAM_DRAW);

	shaderBind(sCloud);
	glEnableVertexAttribArray (0);
	glDisableVertexAttribArray(1);
	glEnableVertexAttribArray (2);
	for(int i=0;i<8;i++){
		matMov(matMVP,matView);
		matMulTrans(matMVP,cloudOffset - parts[i].base,0,0);
		matMul(matMVP,matMVP,matProjection);
		shaderMatrix(sCloud,matMVP);

		glBindBuffer(GL_ARRAY_BUFFER, parts[i].vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT        , GL_FALSE, sizeof(glCloud), (void *)(((char *)&cloudData[0].x) -     ((char *)cloudData)));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glCloud), (void *)(((char *)&cloudData[0].color) - ((char *)cloudData)));
		glDrawArrays(GL_POINTS,0,CLOUDS_MAX - parts[i].count);
	}
	parts[cloudFrame & 7].count = CLOUDS_MAX;

	static uint lastRender = 0;
	if(lastRender == 0){lastRender = getTicks();}
	uint cticks = getTicks();
	cloudOffset += (cticks - lastRender)/2048.f;
	lastRender = cticks;
	if(cloudOffset > 256.f){cloudOffset-=256.f;}
}

void cloudsDraw(u8 cx, u8 cy, u8 cz){
	if(cy&1){return;}
	const u8 cpart = (cx&1) | ((cy&4)) | ((cz&1)<<1);
	if((cloudFrame&7) != cpart){return;}
	cloudChunk *part = &parts[cpart];
	part->base     = floorf(cloudOffset);
	const int toff = (int)cloudOffset;
	const int divx = (int)player->pos.x - (cx<<8);
	const int divz = (int)player->pos.z - (cz<<8);
	const int minx = MIN(divx,255);
	const int maxx = MAX(divx,  0);
	const int minz = MIN(divz,255);
	const int maxz = MAX(divz,  0);
	const ivec cp  = ivecNew(cx<<8,cy<<8,cz<<8);
	const ivec pp  = ivecNewV(player->pos);
	ivec dp        = ivecZero();
	dp.y = (cp.y - pp.y)*(cp.y - pp.y);

	if(maxz<256){
		for(int x=maxx;x<256;x++){
			const int tx = (x-toff)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=maxz;z<256;z++){
				const u8 v = cloudTex[tx][z];
				const int czz = cp.z+z;
				if(v < CLOUD_DENSITY_MIN){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
		for(int x=minx;x>=0;x--){
			const int tx = (x-toff)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=maxz;z<256;z++){
				const int czz = cp.z+z;
				const u8 v = cloudTex[tx][z];
				if(v < CLOUD_DENSITY_MIN){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
	}

	if(minz>0){
		for(int x=maxx;x<256;x++){
			const int tx = (x-toff)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=minz;z>=0;z--){
				const u8 v = cloudTex[tx][z];
				const int czz = cp.z+z;
				if(v < CLOUD_DENSITY_MIN){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
		for(int x=minx;x>=0;x--){
			const int tx = (x-toff)&0xFF;
			const int cxx = cp.x+x;
			dp.x = (cxx - pp.x)*(cxx - pp.x);
			for(int z=minz;z>=0;z--){
				const u8 v = cloudTex[tx][z];
				const int czz = cp.z+z;
				if(v < CLOUD_DENSITY_MIN){ continue; }
				dp.z = (czz - pp.z)*(czz - pp.z);
				cloudPart(part,cxx,cp.y,czz,ivecSum(dp),v);
			}
		}
	}
}

void cloudsCalcColors(){
	static float lastBrightness = 100.f;
	if(fabsf(lastBrightness - skyBrightness) < 0.01f){return;}
	lastBrightness = skyBrightness;
	for(int i=0;i<128;i++){
		const u32 v  = i+128;
		const u32 ta = MIN(255,(218+((256 - v)/2))) * skyBrightness;
		const u32 tb = MIN(255,(178+((256 - v)/4))) * skyBrightness;
		const u32 ba = (164+((256 - v)  ))          * skyBrightness;
		const u32 bb = (148+((256 - v)/2))          * skyBrightness;

		cloudCT[i] = ((tb<<16) | (ta<<8) | ta) & 0x00FFFFFF;
		cloudCB[i] = ((bb<<16) | (ba<<8) | ba) & 0x00FFFFFF;
	}
}

void cloudsInit(){
	generateNoise(0x84407db3, cloudTex);
	for(int i=0;i<8;i++){
		glGenBuffers(1,&parts[i].vbo);
		parts[i].count = CLOUDS_MAX;
		parts[i].base  = 0.f;
	}
	cloudsCalcColors();
}
