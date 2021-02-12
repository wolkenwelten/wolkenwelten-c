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
#include "../game/entity.h"
#include "../game/time.h"
#include "../game/weather.h"
#include "../voxel/chungus.h"
#include "../tmp/assets.h"
#include "../../../common/src/misc/colors.h"

#include <math.h>
#include <stdio.h>

float sunAngle = 45.f;
float skyBrightness;
float worldBrightness;

mesh *sunMesh = NULL;
texture *tSun = NULL;

void initSky(){
	if(sunMesh == NULL){
		sunMesh = meshNew();
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

static void drawSkyColor(){
	skyBrightness = gtimeGetSkyBrightness(gtimeGetTimeOfDay());
	worldBrightness = gtimeGetBrightness(gtimeGetTimeOfDay());

	hsvaColor hsv;
	hsv.h = 160;
	hsv.s = cloudDensityMin - 32;
	hsv.v = (skyBrightness * 231.f) + 24;

	if((skyBrightness < 0.6f) && (skyBrightness > 0.5f)){
		const float bright = 1.f - fabsf((MAX(0.f,(skyBrightness - 0.5f)) * 20.f) - 1.f);
		hsv.h = (((int)(bright * 140.f)) + 160) & 0xFF;
	}

	u32 ccolor = RGBAToU(hsvToRGB(hsv));
	glClearColor( (ccolor&0xFF)/256.f, ((ccolor>>8)&0xFF)/256.f, ((ccolor>>16)&0xFF)/256.f, 1.f );
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void renderSky(const character *cam){
	drawSkyColor();

	shaderBind(sMesh);
	shaderBrightness(sMesh,1.f);
	const vec shake = vecAdd(cam->rot,camShake);
	sunAngle = (((float)gtimeGetTimeOfDay() / (float)(1<<20)) * 360.f)+180.f;
	matIdentity(matMVP);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matSkyProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(sunMesh);
}
