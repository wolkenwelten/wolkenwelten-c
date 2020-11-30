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

#include <math.h>
#include <stdio.h>

float sunAngle = 45.f;
float skyBrightness;

mesh *sunMesh;
texture *tSun;

void initSky(){
	tSun = textureNew(gfx_sun_png_data, gfx_sun_png_len, "client/gfx/sun.png");

	sunMesh = meshNew();
	sunMesh->tex = tSun;
	const float d = 512.f;

	meshAddVert(sunMesh, -48,  d, -48, 0.0f, 0.0f);
	meshAddVert(sunMesh,  48,  d, -48, 1.0f, 0.0f);
	meshAddVert(sunMesh,  48,  d,  48, 1.0f, 1.0f);

	meshAddVert(sunMesh,  48,  d,  48, 1.0f, 1.0f);
	meshAddVert(sunMesh, -48,  d,  48, 0.0f, 1.0f);
	meshAddVert(sunMesh, -48,  d, -48, 0.0f, 0.0f);
	meshFinishStatic(sunMesh);
}

static void drawSkyColor(){
	const float t = .5f + (gtimeGetTimeOfDay() / (float)(1<<20));
	const float tv = t*(PI*2);
	skyBrightness = MIN(1.f,MAX(0.3f,cosf(tv) * 1.5f));
	const float v = skyBrightness;
	glClearColor( 0.33f*v, 0.64f*v, 0.99f*v, 1.f );
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
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(sunMesh);
}
