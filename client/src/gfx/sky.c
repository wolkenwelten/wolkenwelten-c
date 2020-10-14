#include "sky.h"

#include "../gfx/mat.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/mesh.h"
#include "../game/entity.h"
#include "../voxel/chungus.h"
#include "../tmp/assets.h"

#include <math.h>
#include <stdio.h>

float sunAngle = 40.f;

mesh *skyMesh;
mesh *sunMesh;
texture *tSky;
texture *tSun;

void initSky(){
	tSky = textureNew(gfx_sky_png_data, gfx_sky_png_len, "client/gfx/sky.png");
	tSun = textureNew(gfx_sun_png_data, gfx_sun_png_len, "client/gfx/sun.png");

	skyMesh = meshNew();
	skyMesh->tex = tSky;
	const float bv = 640;
	const float sv = 512;

	meshAddVert(skyMesh, -bv,  bv, -bv, 0.250f, 0.00f);
	meshAddVert(skyMesh,  bv,  bv, -bv, 0.500f, 0.00f);
	meshAddVert(skyMesh,  bv,  bv,  bv, 0.500f, 0.33f);

	meshAddVert(skyMesh,  bv,  bv,  bv, 0.500f, 0.33f);
	meshAddVert(skyMesh, -bv,  bv,  bv, 0.250f, 0.33f);
	meshAddVert(skyMesh, -bv,  bv, -bv, 0.250f, 0.00f);

	meshAddVert(skyMesh, -bv, -bv, -bv, 0.500f, 0.66f);
	meshAddVert(skyMesh, -bv, -bv,  bv, 0.500f, 1.00f);
	meshAddVert(skyMesh,  bv, -bv,  bv, 0.250f, 1.00f);

	meshAddVert(skyMesh,  bv, -bv,  bv, 0.250f, 1.00f);
	meshAddVert(skyMesh,  bv, -bv, -bv, 0.250f, 0.66f);
	meshAddVert(skyMesh, -bv, -bv, -bv, 0.500f, 0.66f);



	meshAddVert(skyMesh,   0, -bv,  bv, 0.250f, 0.66f);
	meshAddVert(skyMesh,   0,  bv,  bv, 0.250f, 0.33f);
	meshAddVert(skyMesh,  sv,  bv,  sv, 0.375f, 0.33f);

	meshAddVert(skyMesh,  sv,  bv,  sv, 0.375f, 0.33f);
	meshAddVert(skyMesh,  sv, -bv,  sv, 0.375f, 0.66f);
	meshAddVert(skyMesh,   0, -bv,  bv, 0.250f, 0.66f);

	meshAddVert(skyMesh, -sv, -bv,  sv, 0.375f, 0.66f);
	meshAddVert(skyMesh, -sv,  bv,  sv, 0.375f, 0.33f);
	meshAddVert(skyMesh,   0,  bv,  bv, 0.500f, 0.33f);

	meshAddVert(skyMesh,   0,  bv,  bv, 0.500f, 0.33f);
	meshAddVert(skyMesh,   0, -bv,  bv, 0.500f, 0.66f);
	meshAddVert(skyMesh, -sv, -bv,  sv, 0.375f, 0.66f);


	meshAddVert(skyMesh,   0, -bv, -bv, 0.750f, 0.66f);
	meshAddVert(skyMesh,  sv,  bv, -sv, 0.875f, 0.33f);
	meshAddVert(skyMesh,   0,  bv, -bv, 0.750f, 0.33f);

	meshAddVert(skyMesh,  sv,  bv, -sv, 0.875f, 0.33f);
	meshAddVert(skyMesh,   0, -bv, -bv, 0.750f, 0.66f);
	meshAddVert(skyMesh,  sv, -bv, -sv, 0.875f, 0.66f);

	meshAddVert(skyMesh, -sv, -bv, -sv, 1.000f, 0.66f);
	meshAddVert(skyMesh,   0,  bv, -bv, 0.750f, 0.33f);
	meshAddVert(skyMesh, -sv,  bv, -sv, 0.875f, 0.33f);

	meshAddVert(skyMesh,   0,  bv, -bv, 1.000f, 0.33f);
	meshAddVert(skyMesh, -sv, -bv, -sv, 0.875f, 0.66f);
	meshAddVert(skyMesh,   0, -bv, -bv, 1.000f, 0.66f);


	meshAddVert(skyMesh,  bv, -bv,   0, 0.500f, 0.66f);
	meshAddVert(skyMesh,  sv,  bv,  sv, 0.625f, 0.33f);
	meshAddVert(skyMesh,  bv,  bv,   0, 0.500f, 0.33f);

	meshAddVert(skyMesh,  sv,  bv,  sv, 0.625f, 0.33f);
	meshAddVert(skyMesh,  bv, -bv,   0, 0.500f, 0.66f);
	meshAddVert(skyMesh,  sv, -bv,  sv, 0.625f, 0.66f);

	meshAddVert(skyMesh,  sv, -bv, -sv, 0.625f, 0.66f);
	meshAddVert(skyMesh,  bv,  bv,   0, 0.750f, 0.33f);
	meshAddVert(skyMesh,  sv,  bv, -sv, 0.655f, 0.33f);

	meshAddVert(skyMesh,  bv,  bv,   0, 0.750f, 0.33f);
	meshAddVert(skyMesh,  sv, -bv, -sv, 0.625f, 0.66f);
	meshAddVert(skyMesh,  bv, -bv,   0, 0.750f, 0.66f);


	meshAddVert(skyMesh, -bv, -bv,   0, 0.500f, 0.66f);
	meshAddVert(skyMesh, -bv,  bv,   0, 0.500f, 0.33f);
	meshAddVert(skyMesh, -sv,  bv,  sv, 0.625f, 0.33f);

	meshAddVert(skyMesh, -sv,  bv,  sv, 0.625f, 0.33f);
	meshAddVert(skyMesh, -sv, -bv,  sv, 0.625f, 0.66f);
	meshAddVert(skyMesh, -bv, -bv,   0, 0.500f, 0.66f);

	meshAddVert(skyMesh, -sv, -bv, -sv, 0.625f, 0.66f);
	meshAddVert(skyMesh, -sv,  bv, -sv, 0.655f, 0.33f);
	meshAddVert(skyMesh, -bv,  bv,   0, 0.750f, 0.33f);

	meshAddVert(skyMesh, -bv,  bv,   0, 0.750f, 0.33f);
	meshAddVert(skyMesh, -bv, -bv,   0, 0.750f, 0.66f);
	meshAddVert(skyMesh, -sv, -bv, -sv, 0.625f, 0.66f);
	meshFinishStatic(skyMesh);


	sunMesh = meshNew();
	sunMesh->tex = tSun;

	meshAddVert(sunMesh, -48,  512, -48, 0.0f, 0.0f);
	meshAddVert(sunMesh,  48,  512, -48, 1.0f, 0.0f);
	meshAddVert(sunMesh,  48,  512,  48, 1.0f, 1.0f);

	meshAddVert(sunMesh,  48,  512,  48, 1.0f, 1.0f);
	meshAddVert(sunMesh, -48,  512,  48, 0.0f, 1.0f);
	meshAddVert(sunMesh, -48,  512, -48, 0.0f, 0.0f);
	meshFinishStatic(sunMesh);
}

void renderSky(const character *cam){
	shaderBind(sMesh);

	matIdentity(matMVP);
	const vec shake = vecAdd(cam->rot,camShake);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(skyMesh);

	matIdentity(matMVP);
	matMulRotXY(matMVP,shake.yaw,shake.pitch);
	matMulRotX(matMVP,sunAngle);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDrawLin(sunMesh);
}
