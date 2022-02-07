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

#include "sky.h"

#include "../gfx/mat.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/mesh.h"
#include "../gui/gui.h"
#include "../tmp/meshAssets.h"
#include "../tmp/objs.h"
#include "../game/entity.h"
#include "../game/weather/weather.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/misc/colors.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float sunAngle = 45.f;
float skyBrightness;
float worldBrightness;

mesh *sunMesh = NULL;
texture *tSun = NULL;
texture *tSky = NULL;
mesh    *mSky = NULL;

extern uint  gfx_sun_png_len;
extern uchar gfx_sun_png_data[];

u32 *skyTextureBuffer;
int skyTextureSize = 16;

void initSky(){
	mSky = meshNewAsset("skydome", &skydome_meshdata);
	tSky = textureNewRaw();
	tSky->w = skyTextureSize;
	tSky->h = skyTextureSize;
	tSky->d = 0;
	mSky->tex = tSky;
	meshFinishStatic( mSky );

	if(sunMesh == NULL){
		sunMesh = meshNew("sun");
	}
	meshEmpty(sunMesh);
	if(tSun == NULL){
		tSun = textureNew(gfx_sun_png_data, gfx_sun_png_len, "client/gfx/sun.png");
		sunMesh->tex = tSun;
	}
	const float d = 2048.f;

	meshAddVert(sunMesh, -128,  d, -128, 0.0f, 0.0f);
	meshAddVert(sunMesh,  128,  d, -128, 1.0f, 0.0f);
	meshAddVert(sunMesh,  128,  d,  128, 1.0f, 1.0f);

	meshAddVert(sunMesh,  128,  d,  128, 1.0f, 1.0f);
	meshAddVert(sunMesh, -128,  d,  128, 0.0f, 1.0f);
	meshAddVert(sunMesh, -128,  d, -128, 0.0f, 0.0f);
	meshFinishStatic(sunMesh);
}

static void getSkyColors(u32 *hi, u32 *lo){
	skyBrightness = gtimeGetSkyBrightness(gtimeGetTimeOfDay());
	worldBrightness = gtimeGetBrightness(gtimeGetTimeOfDay());

	hsvaColor hsv;
	hsv.h = 160;
	hsv.s = cloudDensityMin - 32;
	hsv.v = (skyBrightness * 192.f) + 62;

	if((skyBrightness < 0.6f) && (skyBrightness > 0.5f)){
		const float bright = 1.f - fabsf((MAX(0.f,(skyBrightness - 0.5f)) * 20.f) - 1.f);
		hsv.h = (((int)(bright * 140.f)) + 160) & 0xFF;
	}

	*hi = RGBAToU(hsvToRGB(hsv)) | 0xFF000000;
	hsv.v = MAX(8,hsv.v-80);
	hsv.s = MIN(255,hsv.s+120);
	*lo = RGBAToU(hsvToRGB(hsv)) | 0xFF000000;
}

static void genSkyTexture(){
	static u32 lastHi = 0;
	if(skyTextureBuffer == NULL){
		skyTextureBuffer = malloc(skyTextureSize * skyTextureSize * sizeof(u32));
	}
	if(skyTextureBuffer == NULL){return;}
	u32 hi,lo;
	getSkyColors(&hi,&lo);
	if(hi == lastHi){return;}
	lastHi = hi;
	u32 *p = skyTextureBuffer;

	for(int y = 0; y < skyTextureSize; y++){
		const u32 c = colorInterpolateRGB(hi,lo,abs(y-skyTextureSize/2) / (skyTextureSize/2.f));
		for(int x = 0; x < skyTextureSize; x++){
			*p++ = c;
		}
	}
	textureLoadSurface(tSky,skyTextureSize,skyTextureSize,skyTextureBuffer,true);
}


void renderSky(const character *cam){
	gfxGroupStart("Sky");
	glClear(GL_DEPTH_BUFFER_BIT);
	genSkyTexture();

	shaderBind(sMesh);
	shaderColor(sMesh, 1.f, 1.f, 1.f, 1.f);
	const vec shake = vecAdd(cam->rot,camShake);
	matIdentity(matMVP);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMulScale (matMVP,renderDistance,renderDistance,renderDistance);

	matMul(matMVP,matMVP,matSkyProjection);
	shaderMatrix(sMesh,matMVP);
	glDepthMask(GL_FALSE);
	meshDraw(mSky);

	sunAngle = (((float)gtimeGetTimeOfDay() / (float)(1<<20)) * 360.f)+180.f;
	matIdentity(matMVP);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matSkyProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(sunMesh);
	glDepthMask(GL_TRUE);
	gfxGroupEnd();
}
