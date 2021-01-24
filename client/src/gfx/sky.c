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
#include "../voxel/chungus.h"
#include "../tmp/assets.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>

float sunAngle = 45.f;
float skyBrightness;
float worldBrightness;

mesh *sunMesh = NULL;
texture *tSun = NULL;

u32 skyColorDawn  = 0xFF419CF8;
u32 skyColorDay   = 0xFFF89C41;
u32 skyColorNight = 0xFF1A0C05;

void initSky(){
	if(sunMesh == NULL){
		sunMesh = meshNew();
	}
	meshEmpty(sunMesh);
	if(tSun == NULL){
		tSun = textureNew(gfx_sun_png_data, gfx_sun_png_len, "client/gfx/sun.png");
		sunMesh->tex = tSun;
	}
	const float d = renderDistance+24;

	meshAddVert(sunMesh, -48,  d, -48, 0.0f, 0.0f);
	meshAddVert(sunMesh,  48,  d, -48, 1.0f, 0.0f);
	meshAddVert(sunMesh,  48,  d,  48, 1.0f, 1.0f);

	meshAddVert(sunMesh,  48,  d,  48, 1.0f, 1.0f);
	meshAddVert(sunMesh, -48,  d,  48, 0.0f, 1.0f);
	meshAddVert(sunMesh, -48,  d, -48, 0.0f, 0.0f);
	meshFinishStatic(sunMesh);
}

static void drawSkyColor(){
	skyBrightness = gtimeGetSkyBrightness(gtimeGetTimeOfDay());
	worldBrightness = gtimeGetBrightness(gtimeGetTimeOfDay());
	u32 ccolor;
	const u32 scolor = colorInterpolate(skyColorNight,skyColorDay,skyBrightness);

	if(skyBrightness > 0.8f){
		const float bright = 1.f - fabsf((MAX(0.f,(skyBrightness - 0.8f)) * 10.f) - 1.f);
		ccolor = colorInterpolate(scolor,skyColorDawn,bright);
	}else{
		ccolor = scolor;
	}

	glClearColor( (ccolor&0xFF)/256.f, ((ccolor>>8)&0xFF)/256.f, ((ccolor>>16)&0xFF)/256.f, 1.f );
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void renderSky(const character *cam){
	static float lastRD = 0;
	if(fabsf(lastRD - renderDistance) > .1f){initSky();}
	drawSkyColor();

	shaderBind(sMesh);
	shaderBrightness(sMesh,1.f);
	const vec shake = vecAdd(cam->rot,camShake);
	sunAngle = (((float)gtimeGetTimeOfDay() / (float)(1<<20)) * 360.f)+180.f;
	matIdentity(matMVP);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(sunMesh);
}
