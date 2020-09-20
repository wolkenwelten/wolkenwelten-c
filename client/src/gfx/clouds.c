#include "clouds.h"

#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/particle.h"
#include "../game/character.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/noise.h"

#include <stdio.h>
#include <math.h>


#pragma pack(push, 1)
typedef struct glParticle {
	float x,y,z;
	unsigned int color;
} glCloud;
#pragma pack(pop)

#define CLOUD_FADED (65536)
#define CLOUD_MIND  (65536*3)
#define CLOUD_DENSITY_MIN 168
#define CLOUDS_MAX (1<<19)

u8         cloudTex[256][256];
float      cloudOffset = 0.f;

glCloud    glData[CLOUDS_MAX];
uint       glCount  = CLOUDS_MAX;
uint       cloudVBO = 0;


static inline void cloudPart(float px,float py,float pz,float dd,u8 v){
	if(dd > (CLOUD_MIND+CLOUD_FADED)){return;}
	if(glCount < 4){return;}

	const float   vf = v-170;
	const u8 ta = (218+((256 - v)/3));
	const u8 tb = (198+((256 - v)/6));
	const u8 ba = (164+((256 - v)  ));
	const u8 bb = (148+((256 - v)/2));
	u32 a;
	if(dd > CLOUD_MIND){
		a = (u8)(v*(1.f-((dd - CLOUD_MIND)/CLOUD_FADED))) << 24;
	}else{
		a = v << 24;
	}
	const u32 ct = a | (tb<<16) | (ta<<8) | ta;
	const u32 cb = a | (bb<<16) | (ba<<8) | ba;
	const float oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED)) * 23.f + 19.f;
	py += oy;

	if(py < player->pos.y){
		glData[--glCount] = (glCloud){px,py+vf/ 6.f,pz,ct};
		glData[--glCount] = (glCloud){px,py+vf/18.f,pz,cb};
		glData[--glCount] = (glCloud){px,py-vf/12.f,pz,cb};
	}else{
		glData[--glCount] = (glCloud){px,py-vf/12.f,pz,cb};
		glData[--glCount] = (glCloud){px,py+vf/18.f,pz,cb};
		glData[--glCount] = (glCloud){px,py+vf/ 6.f,pz,ct};
	}
}

void cloudsRender(){
	shaderBind(sCloud);
	matMov(matMVP,matView);
	matMulTrans(matMVP,cloudOffset-floorf(cloudOffset),0,0);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sCloud,matMVP);

	glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
	glBufferData(GL_ARRAY_BUFFER, (CLOUDS_MAX-glCount)*sizeof(glCloud), &glData[glCount], GL_STREAM_DRAW);

	glEnableVertexAttribArray (0);
	glDisableVertexAttribArray(1);
	glEnableVertexAttribArray (2);

	glVertexAttribPointer(0, 3, GL_FLOAT        , GL_FALSE, sizeof(glCloud), (void *)(((char *)&glData[0].x) -     ((char *)glData)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glCloud), (void *)(((char *)&glData[0].color) - ((char *)glData)));
	glDrawArrays(GL_POINTS,0,CLOUDS_MAX-glCount);
	glCount = CLOUDS_MAX;
}

void cloudsDraw(int cx, int cy, int cz){
	if(cy&1){return;}
	const int toff = (int)cloudOffset;
	const int divx = (int)player->pos.x - (cx<<8);
	const int divz = (int)player->pos.z - (cz<<8);
	const ivec cp  = ivecNew(cx<<8,cy<<8,cz<<8);
	const ivec pp  = ivecNewV(player->pos);
	ivec dp        = ivecZero();
	dp.y = (cp.y - pp.y)*(cp.y - pp.y);

	for(int x=MAX(divx,0);x<256;x++){
		const int tx = (x-toff)&0xFF;
		const int cxx = cp.x+x;
		dp.x = (cxx - pp.x)*(cxx - pp.x);
		for(int z=MAX(divz,0);z<256;z++){
			const u8 v = cloudTex[tx][z];
			const int czz = cp.z+z;
			if(v < CLOUD_DENSITY_MIN){ continue; }
			dp.z = (czz - pp.z)*(czz - pp.z);
			cloudPart(cxx,cp.y,czz,ivecSum(dp),v);
		}
	}

	for(int x=MIN(256,divx)-1;x>=0;x--){
		const int tx = (x-toff)&0xFF;
		const int cxx = cp.x+x;
		dp.x = (cxx - pp.x)*(cxx - pp.x);
		for(int z=MAX(divz,0);z<256;z++){
			const int czz = cp.z+z;
			const u8 v = cloudTex[tx][z];
			if(v < CLOUD_DENSITY_MIN){ continue; }
			dp.z = (czz - pp.z)*(czz - pp.z);
			cloudPart(cxx,cp.y,czz,ivecSum(dp),v);
		}
	}

	for(int x=MAX(divx,0);x<256;x++){
		const int tx = (x-toff)&0xFF;
		const int cxx = cp.x+x;
		dp.x = (cxx - pp.x)*(cxx - pp.x);
		for(int z=MIN(256,divz)-1;z>=0;z--){
			const u8 v = cloudTex[tx][z];
			const int czz = cp.z+z;
			if(v < CLOUD_DENSITY_MIN){ continue; }
			dp.z = (czz - pp.z)*(czz - pp.z);
			cloudPart(cxx,cp.y,czz,ivecSum(dp),v);
		}
	}

	for(int x=MIN(256,divx)-1;x>=0;x--){
		const int tx = (x-toff)&0xFF;
		const int cxx = cp.x+x;
		dp.x = (cxx - pp.x)*(cxx - pp.x);
		for(int z=MIN(256,divz)-1;z>=0;z--){
			const u8 v = cloudTex[tx][z];
			const int czz = cp.z+z;
			if(v < CLOUD_DENSITY_MIN){ continue; }
			dp.z = (czz - pp.z)*(czz - pp.z);
			cloudPart(cxx,cp.y,czz,ivecSum(dp),v);
		}
	}
}

void cloudsInit(){
	generateNoise(0x84407db3, cloudTex);
	glGenBuffers(1,&cloudVBO);
}
