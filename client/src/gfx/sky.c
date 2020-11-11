#include "sky.h"

#include "../gfx/mat.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/mesh.h"
#include "../gui/gui.h"
#include "../game/entity.h"
#include "../voxel/chungus.h"
#include "../tmp/assets.h"

#include <math.h>
#include <stdio.h>

float sunAngle = 45.f;

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

void renderSky(const character *cam){
	shaderBind(sMesh);
	const vec shake = vecAdd(cam->rot,camShake);
	matIdentity(matMVP);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(sunMesh);
}
