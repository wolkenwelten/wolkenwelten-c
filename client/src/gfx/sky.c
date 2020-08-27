#include "sky.h"

#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/mesh.h"
#include "../game/entity.h"
#include "../tmp/assets.h"

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
