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


void cloudPart(float px,float py,float pz,float dd,u8 v){
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
	matMul(matMVP,matView,matProjection);
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
	const int toff    = cloudOffset;
	float sx = cx * 256.f + cloudOffset - toff;
	float sy = cy * 256.f;
	float sz = cz * 256.f;
	if(cy&1){return;}
	const float dy = (sy - player->pos.y) * (sy - player->pos.y);

	const int divx = (int)player->pos.x - (cx<<8);
	const int divz = (int)player->pos.z - (cz<<8);

	for(int x=MAX(divx,0);x<256;x++){
		const float px  = sx+x;
		const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
		const int tx    = (x - toff)&0xFF;
		for(int z=MAX(divz,0);z<256;z++){
			const u8       v = cloudTex[tx][z];
			if(v < CLOUD_DENSITY_MIN){ continue; }
			const float   pz = sz+z;
			const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
			const float   dd = dxy+dz;
			if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
			const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
			cloudPart(px,sy+oy,pz,dd,v);
		}
	}

	for(int x=MIN(256,divx)-1;x>=0;x--){
		const float px  = sx+x;
		const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
		const int tx    = (x - toff)&0xFF;
		for(int z=MAX(divz,0);z<256;z++){
			const u8      v = cloudTex[tx][z];
			if(v < CLOUD_DENSITY_MIN){ continue; }
			const float  pz = sz+z;
			const float  dz = (pz - player->pos.z) * (pz - player->pos.z);
			const float  dd = dxy+dz;
			if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
			const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
			cloudPart(px,sy+oy,pz,dd,v);
		}
	}

	for(int x=MAX(divx,0);x<256;x++){
		const float px  = sx+x;
		const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
		const int tx    = (x - toff)&0xFF;
		for(int z=MIN(256,divz)-1;z>=0;z--){
			const u8       v = cloudTex[tx][z];
			if(v < CLOUD_DENSITY_MIN){ continue; }
			const float   pz = sz+z;
			const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
			const float   dd = dxy+dz;
			if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
			const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
			cloudPart(px,sy+oy,pz,dd,v);
		}
	}

	for(int x=MIN(256,divx)-1;x>=0;x--){
		const float px  = sx+x;
		const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
		const int tx    = (x - toff)&0xFF;
		for(int z=MIN(256,divz)-1;z>=0;z--){
			const u8       v = cloudTex[tx][z];
			if(v < CLOUD_DENSITY_MIN){ continue; }
			const float   pz = sz+z;
			const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
			const float   dd = dxy+dz;
			if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
			const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
			cloudPart(px,sy+oy,pz,dd,v);
		}
	}
}

void cloudsInit(){
	generateNoise(0x84407db3, cloudTex);
	glGenBuffers(1,&cloudVBO);
}
