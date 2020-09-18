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

typedef struct {
	float x,z,d;
	uint16_t y;
	uint16_t v;
}cloudEntry;

#define CLOUD_FADED (65536)
#define CLOUD_MIND  (65536*3)
#define CLOUD_DENSITY_MIN 168
#define CLOUDS_MAX (1<<19)

cloudEntry clouds[1<<14];
uint       cloudCount  = 0;
uint8_t    cloudTex[256][256];
float      cloudOffset = 0.f;

glCloud    glData[CLOUDS_MAX];
uint       glCount  = 0;
uint       cloudVBO = 0;

int quicksortCloudsPart(int lo, int hi){
	float p = clouds[hi].d;
	int i   = lo;
	for(int j = lo;j<=hi;j++){
		if(clouds[j].d < p){
			cloudEntry t = clouds[i];
			clouds[i]    = clouds[j];
			clouds[j]    = t;
			i++;
		}
	}
	cloudEntry t = clouds[i];
	clouds[i]    = clouds[hi];
	clouds[hi]   = t;
	return i;
}

void quicksortClouds(int lo, int hi){
	if(lo >= hi){ return; }
	int p = quicksortCloudsPart(lo,hi);
	quicksortClouds(lo , p-1);
	quicksortClouds(p+1, hi);
}

void cloudPart(float px,float py,float pz,float dd,uint8_t v){
	const float   vf = v-170;
	const uint8_t ta = (218+((256 - v)/3));
	const uint8_t tb = (198+((256 - v)/6));
	const uint8_t ba = (164+((256 - v)  ));
	const uint8_t bb = (148+((256 - v)/2));
	uint32_t a;
	if(dd > CLOUD_MIND){
		a = (uint8_t)(v*(1.f-((dd - CLOUD_MIND)/CLOUD_FADED))) << 24;
	}else{
		a = v << 24;
	}
	const uint32_t ct = a | (tb<<16) | (ta<<8) | ta;
	const uint32_t cb = a | (bb<<16) | (ba<<8) | ba;

	if(py > player->pos.y){
		glData[glCount++] = (glCloud){px,py+vf/ 6.f,pz,ct};
		glData[glCount++] = (glCloud){px,py+vf/18.f,pz,cb};
		glData[glCount++] = (glCloud){px,py-vf/12.f,pz,cb};
	}else{
		glData[glCount++] = (glCloud){px,py-vf/12.f,pz,cb};
		glData[glCount++] = (glCloud){px,py+vf/18.f,pz,cb};
		glData[glCount++] = (glCloud){px,py+vf/ 6.f,pz,ct};
	}
}

void cloudsRenderGl(){
	shaderBind(sCloud);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sCloud,matMVP);

	glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
	glBufferData(GL_ARRAY_BUFFER, glCount*sizeof(glCloud), glData, GL_STREAM_DRAW);

	glEnableVertexAttribArray (0);
	glDisableVertexAttribArray(1);
	glEnableVertexAttribArray (2);

	glVertexAttribPointer(0, 3, GL_FLOAT        , GL_FALSE, sizeof(glCloud), (void *)(((char *)&glData[0].x) -     ((char *)glData)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(glCloud), (void *)(((char *)&glData[0].color) - ((char *)glData)));
	glDrawArrays(GL_POINTS,0,glCount);
}

void cloudsRender(){
	quicksortClouds(0,cloudCount);

	for(int i=cloudCount-1;i>=0;i--){
		const uint8_t  v = clouds[i].v;
		const float   dd = clouds[i].d;
		const float   px = clouds[i].x;
		const float   pz = clouds[i].z;
		const float   sy = clouds[i].y<<8;
		const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
		cloudPart(px,sy+oy,pz,dd,v);
	}
	cloudsRenderGl();

	glCount = 0;
	cloudCount = 0;
}

void cloudsDrawDeferred(int cx, int cy, int cz){
	const int toff    = cloudOffset;
	float sx = cx * 256.f + cloudOffset - toff;
	float sy = cy * 256.f;
	float sz = cz * 256.f;
	if(cy&1){return;}
	const float dy = (sy - player->pos.y) * (sy - player->pos.y);

	for(uint x=0;x<256;x++){
		const float px  = sx+x;
		const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
		const int tx    = (x - toff)&0xFF;
		for(uint z=0;z<256;z++){
			const uint8_t  v = cloudTex[tx][z];
			if(v < CLOUD_DENSITY_MIN){ continue; }
			const float   pz = sz+z;
			const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
			const float   dd = dxy+dz;
			if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
			clouds[cloudCount++] = (cloudEntry){px,pz,dd,cy,v};
		}
	}
}

void cloudsDrawDirect(int cx, int cy, int cz){
	const int toff    = cloudOffset;
	float sx = cx * 256.f + cloudOffset - toff;
	float sy = cy * 256.f;
	float sz = cz * 256.f;
	if(cy&1){return;}
	const float dy = (sy - player->pos.y) * (sy - player->pos.y);
	uint8_t dir = 0;
	if(sx > player->pos.x){dir |= 1;}
	if(sz > player->pos.z){dir |= 2;}

	switch(dir){
	default:
	case 0:
		for(int x=0;x<256;x++){
			const float px  = sx+x;
			const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
			const int tx    = (x - toff)&0xFF;
			for(int z=0;z<256;z++){
				const uint8_t  v = cloudTex[tx][z];
				if(v < CLOUD_DENSITY_MIN){ continue; }
				const float   pz = sz+z;
				const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
				const float   dd = dxy+dz;
				if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
				const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
				cloudPart(px,sy+oy,pz,dd,v);
			}
		}
		break;
	case 1:
		for(int x=255;x>=0;x--){
			const float px  = sx+x;
			const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
			const int tx    = (x - toff)&0xFF;
			for(int z=0;z<256;z++){
				const uint8_t  v = cloudTex[tx][z];
				if(v < CLOUD_DENSITY_MIN){ continue; }
				const float   pz = sz+z;
				const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
				const float   dd = dxy+dz;
				if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
				const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
				cloudPart(px,sy+oy,pz,dd,v);
			}
		}
		break;
	case 2:
		for(int x=0;x<256;x++){
			const float px  = sx+x;
			const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
			const int tx    = (x - toff)&0xFF;
			for(int z=255;z>=0;z--){
				const uint8_t  v = cloudTex[tx][z];
				if(v < CLOUD_DENSITY_MIN){ continue; }
				const float   pz = sz+z;
				const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
				const float   dd = dxy+dz;
				if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
				const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
				cloudPart(px,sy+oy,pz,dd,v);
			}
		}
		break;

	case 3:
		for(int x=255;x>=0;x--){
			const float px  = sx+x;
			const float dxy = dy + ((px - player->pos.x) * (px - player->pos.x));
			const int tx    = (x - toff)&0xFF;
			for(int z=255;z>=0;z--){
				const uint8_t  v = cloudTex[tx][z];
				if(v < CLOUD_DENSITY_MIN){ continue; }
				const float   pz = sz+z;
				const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
				const float   dd = dxy+dz;
				if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
				const float   oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;
				cloudPart(px,sy+oy,pz,dd,v);
			}
		}
		break;
	}

}

void cloudsDraw(int cx, int cy, int cz){
	const int pcx = (int)player->pos.x >> 8;
	const int pcy = (int)player->pos.y >> 8;
	const int pcz = (int)player->pos.z >> 8;

	if((pcx==cx) && (pcy==cy) && (pcz==cz)){
		cloudsDrawDeferred(cx,cy,cz);
	}else{
		cloudsDrawDirect(cx,cy,cz);
	}
}

void cloudsInit(){
	generateNoise(0x84407db3, cloudTex);
	glGenBuffers(1,&cloudVBO);
}
