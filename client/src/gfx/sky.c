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

uint8_t cloudTex[256][256];
float cloudOffset=0.f;

void cloudsDraw(int cx, int cy, int cz){
	//static int densityTicks = 0;
	const int density = 178;
	const int toff = (int)cloudOffset;
	float sx=cx * 256.f + (cloudOffset - (float)toff);
	float sy=cy * 256.f;
	float sz=cz * 256.f;

	//density = density + (sin(++densityTicks/(4096.0*32))*16);

	if(cy&1){return;}


	const int ttl=1;
	const int MUL=1;

	for(int x=0;x<256;x++){
		const float px = sx+((float)(x*MUL));
		const int tx = (x - toff)&0xFF;
		for(int z=0;z<256;z++){
			const float pz = sz+((float)(z*MUL));
			const uint8_t v  = cloudTex[tx][z];
			const uint8_t vt = 256-((256 - v)/4);
			const uint8_t vb = 256-((256 - v)/2);
			const uint32_t topColor = v << 24 | (vt << 16) | (vt << 8) | vt;
			const uint32_t botColor = v << 24 | (vb << 16) | (vb << 8) | vb;

			if(v > density){
				newParticle(px,sy                      ,pz,0,0,0,0,0,0,2048.f,0.5f,topColor,ttl);
				newParticle(px,sy+((float)(v-170)/ 6.f),pz,0,0,0,0,0,0,2048.f,0.5f,topColor,ttl);
				newParticle(px,sy-((float)(v-170)/12.f),pz,0,0,0,0,0,0,1024.f,0.5f,botColor,ttl);
			}

			/*
			if(v > 248){
				newParticle(px,sy+((float)(v-170)/ 6.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xE8F8F8F8,ttl);
				newParticle(px,sy-((float)(v-170)/18.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xE0F8F8F8,ttl);
			}else if(v > 240){
				newParticle(px,sy+((float)(v-170)/ 6.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xE4E8E8E8,ttl);
				newParticle(px,sy-((float)(v-170)/18.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xE0E0E0E0,ttl);
			}else if(v > 224){
				newParticle(px,sy+((float)(v-170)/ 6.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xD4E8E8E8,ttl);
				newParticle(px,sy-((float)(v-170)/18.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xD0E0E0E0,ttl);
			}else if(v > 192){
				newParticle(px,sy+((float)(v-170)/ 6.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xB2E8E8E8,ttl);
				newParticle(px,sy-((float)(v-170)/18.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xB0E0E0E0,ttl);
			}else if(v > 170){
				newParticle(px,sy+((float)(v-170)/ 6.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xA2D8D8D8,ttl);
				newParticle(px,sy-((float)(v-170)/18.f),pz,0,0,0,0,0,0,1024.f,0.5f,0xA0D0D0D0,ttl);
			}else if(v > 164){
				newParticle(px,sy                      ,pz,0,0,0,0,0,0, 768.f,0.5f,0x90C8C8C8,ttl);
			}*/
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
	float matMVP[16];
	shaderBind(sMesh);

	matIdentity(matMVP);
	matMulRotXY(matMVP,cam->yaw,cam->pitch);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(skyMesh);


	matIdentity(matMVP);
	matMulRotXY(matMVP,cam->yaw,cam->pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(sunMesh);
}
