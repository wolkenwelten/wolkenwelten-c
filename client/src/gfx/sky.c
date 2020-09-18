#include "sky.h"

#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/particle.h"
#include "../gfx/texture.h"
#include "../gfx/mesh.h"
#include "../game/entity.h"
#include "../voxel/chungus.h"
#include "../tmp/assets.h"
#include "../../../common/src/misc/noise.h"

#include <math.h>
#include <stdio.h>

float sunAngle = 40.f;

mesh *skyMesh;
mesh *sunMesh;
texture *tSky;
texture *tSun;

typedef struct {
	float x,z,d;
	uint16_t y;
	uint16_t v;
}cloudEntry;

cloudEntry clouds[1<<20];
uint cloudCount=0;

uint8_t cloudTex[256][256];
float cloudOffset=0.f;

#define CLOUD_FADED (65536)
#define CLOUD_MIND  (65536*3)

int quicksortCloudsPart(int lo, int hi){
	float p = clouds[hi].d;
	int i = lo;
	for(int j = lo;j<=hi;j++){
		if(clouds[j].d < p){
			cloudEntry t = clouds[i];
			clouds[i] = clouds[j];
			clouds[j] = t;
			i++;
		}
	}
	cloudEntry t = clouds[i];
	clouds[i] = clouds[hi];
	clouds[hi] = t;
	return i;
}

void quicksortClouds(int lo, int hi){
	if(lo >= hi){ return; }
	int p = quicksortCloudsPart(lo,hi);
	quicksortClouds(lo , p-1);
	quicksortClouds(p+1, hi);
}

void cloudsRender(){
	quicksortClouds(0,cloudCount);

	for(int i=cloudCount-1;i>=0;i--){
		const uint8_t  v = clouds[i].v;
		const float   dd = clouds[i].d;
		const float   px = clouds[i].x;
		const float   pz = clouds[i].z;
		const float   sy = clouds[i].y<<8;
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
		const float    oy = (1.f - dd / (CLOUD_MIND+CLOUD_FADED))*32.f + 32.f;

		newParticle(px,sy+vf/18.f+oy,pz,0,0,0,0,0,0,1024,0,cb,1);
		newParticle(px,sy-vf/12.f+oy,pz,0,0,0,0,0,0,1024,0,cb,1);
		newParticle(px,sy+vf/ 6.f+oy,pz,0,0,0,0,0,0,1024,0,ct,1);
	}

	cloudCount = 0;
}

void cloudsDraw(int cx, int cy, int cz){
	const int density = 168;
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
			if(v < density){ continue; }
			const float   pz = sz+z;
			const float   dz = (pz - player->pos.z) * (pz - player->pos.z);
			const float   dd = dxy+dz;
			if(dd > (CLOUD_MIND+CLOUD_FADED)){continue;}
			clouds[cloudCount++] = (cloudEntry){px,pz,dd,cy,v};
		}
	}
}


void initSky(){
	generateNoise(0x84407db3, cloudTex);
	tSky = textureNew(gfx_sky_png_data, gfx_sky_png_len, "client/gfx/sky.png");
	tSun = textureNew(gfx_sun_png_data, gfx_sun_png_len, "client/gfx/sun.png");

	skyMesh = meshNew();
	skyMesh->tex = tSky;

	meshAddVert(skyMesh, -1536,  1536, -1536, 0.250f, 0.00f);
	meshAddVert(skyMesh,  1536,  1536, -1536, 0.500f, 0.00f);
	meshAddVert(skyMesh,  1536,  1536,  1536, 0.500f, 0.33f);

	meshAddVert(skyMesh,  1536,  1536,  1536, 0.500f, 0.33f);
	meshAddVert(skyMesh, -1536,  1536,  1536, 0.250f, 0.33f);
	meshAddVert(skyMesh, -1536,  1536, -1536, 0.250f, 0.00f);

	meshAddVert(skyMesh, -1536, -1536, -1536, 0.500f, 0.66f);
	meshAddVert(skyMesh, -1536, -1536,  1536, 0.500f, 1.00f);
	meshAddVert(skyMesh,  1536, -1536,  1536, 0.250f, 1.00f);

	meshAddVert(skyMesh,  1536, -1536,  1536, 0.250f, 1.00f);
	meshAddVert(skyMesh,  1536, -1536, -1536, 0.250f, 0.66f);
	meshAddVert(skyMesh, -1536, -1536, -1536, 0.500f, 0.66f);



	meshAddVert(skyMesh,     0, -1536,  1536, 0.250f, 0.66f);
	meshAddVert(skyMesh,     0,  1536,  1536, 0.250f, 0.33f);
	meshAddVert(skyMesh,  1024,  1536,  1024, 0.375f, 0.33f);

	meshAddVert(skyMesh,  1024,  1536,  1024, 0.375f, 0.33f);
	meshAddVert(skyMesh,  1024, -1536,  1024, 0.375f, 0.66f);
	meshAddVert(skyMesh,     0, -1536,  1536, 0.250f, 0.66f);

	meshAddVert(skyMesh, -1024, -1536,  1024, 0.375f, 0.66f);
	meshAddVert(skyMesh, -1024,  1536,  1024, 0.375f, 0.33f);
	meshAddVert(skyMesh,     0,  1536,  1536, 0.500f, 0.33f);

	meshAddVert(skyMesh,     0,  1536,  1536, 0.500f, 0.33f);
	meshAddVert(skyMesh,     0, -1536,  1536, 0.500f, 0.66f);
	meshAddVert(skyMesh, -1024, -1536,  1024, 0.375f, 0.66f);


	meshAddVert(skyMesh,     0, -1536, -1536, 0.750f, 0.66f);
	meshAddVert(skyMesh,  1024,  1536, -1024, 0.875f, 0.33f);
	meshAddVert(skyMesh,     0,  1536, -1536, 0.750f, 0.33f);

	meshAddVert(skyMesh,  1024,  1536, -1024, 0.875f, 0.33f);
	meshAddVert(skyMesh,     0, -1536, -1536, 0.750f, 0.66f);
	meshAddVert(skyMesh,  1024, -1536, -1024, 0.875f, 0.66f);

	meshAddVert(skyMesh, -1024, -1536, -1024, 1.000f, 0.66f);
	meshAddVert(skyMesh,     0,  1536, -1536, 0.750f, 0.33f);
	meshAddVert(skyMesh, -1024,  1536, -1024, 0.875f, 0.33f);

	meshAddVert(skyMesh,     0,  1536, -1536, 1.000f, 0.33f);
	meshAddVert(skyMesh, -1024, -1536, -1024, 0.875f, 0.66f);
	meshAddVert(skyMesh,     0, -1536, -1536, 1.000f, 0.66f);


	meshAddVert(skyMesh,  1536, -1536,     0, 0.500f, 0.66f);
	meshAddVert(skyMesh,  1024,  1536,  1024, 0.625f, 0.33f);
	meshAddVert(skyMesh,  1536,  1536,     0, 0.500f, 0.33f);

	meshAddVert(skyMesh,  1024,  1536,  1024, 0.625f, 0.33f);
	meshAddVert(skyMesh,  1536, -1536,     0, 0.500f, 0.66f);
	meshAddVert(skyMesh,  1024, -1536,  1024, 0.625f, 0.66f);

	meshAddVert(skyMesh,  1024, -1536, -1024, 0.625f, 0.66f);
	meshAddVert(skyMesh,  1536,  1536,     0, 0.750f, 0.33f);
	meshAddVert(skyMesh,  1024,  1536, -1024, 0.655f, 0.33f);

	meshAddVert(skyMesh,  1536,  1536,     0, 0.750f, 0.33f);
	meshAddVert(skyMesh,  1024, -1536, -1024, 0.625f, 0.66f);
	meshAddVert(skyMesh,  1536, -1536,     0, 0.750f, 0.66f);


	meshAddVert(skyMesh, -1536, -1536,     0, 0.500f, 0.66f);
	meshAddVert(skyMesh, -1536,  1536,     0, 0.500f, 0.33f);
	meshAddVert(skyMesh, -1024,  1536,  1024, 0.625f, 0.33f);

	meshAddVert(skyMesh, -1024,  1536,  1024, 0.625f, 0.33f);
	meshAddVert(skyMesh, -1024, -1536,  1024, 0.625f, 0.66f);
	meshAddVert(skyMesh, -1536, -1536,     0, 0.500f, 0.66f);

	meshAddVert(skyMesh, -1024, -1536, -1024, 0.625f, 0.66f);
	meshAddVert(skyMesh, -1024,  1536, -1024, 0.655f, 0.33f);
	meshAddVert(skyMesh, -1536,  1536,     0, 0.750f, 0.33f);

	meshAddVert(skyMesh, -1536,  1536,     0, 0.750f, 0.33f);
	meshAddVert(skyMesh, -1536, -1536,     0, 0.750f, 0.66f);
	meshAddVert(skyMesh, -1024, -1536, -1024, 0.625f, 0.66f);
	meshFinish(skyMesh, GL_STATIC_DRAW);


	sunMesh = meshNew();
	sunMesh->tex = tSun;

	meshAddVert(sunMesh, -48,  512, -48, 0.0f, 0.0f);
	meshAddVert(sunMesh,  48,  512, -48, 1.0f, 0.0f);
	meshAddVert(sunMesh,  48,  512,  48, 1.0f, 1.0f);

	meshAddVert(sunMesh,  48,  512,  48, 1.0f, 1.0f);
	meshAddVert(sunMesh, -48,  512,  48, 0.0f, 1.0f);
	meshAddVert(sunMesh, -48,  512, -48, 0.0f, 0.0f);
	meshFinish(sunMesh, GL_STATIC_DRAW);
}

void renderSky(const character *cam){
	shaderBind(sMesh);

	matIdentity(matMVP);
	matMulRotXY(matMVP,cam->rot.yaw,cam->rot.pitch);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(skyMesh);


	matIdentity(matMVP);
	matMulRotXY(matMVP,cam->rot.yaw,cam->rot.pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(sunMesh);
}
