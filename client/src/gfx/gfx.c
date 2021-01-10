#include "../gfx/gfx.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/projectile.h"
#include "../game/rain.h"
#include "../game/rope.h"
#include "../game/water.h"
#include "../game/weather.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/sky.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../misc/options.h"
#include "../sdl/sdl.h"
#include "../tmp/assets.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/hook.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


float matProjection[16], matView[16];

int    screenWidth  = 800;
int    screenHeight = 600;
size_t vboTrisCount = 0;
float  gfxCurFOV    = 80.0f;
vec    camShake;


#ifdef __EMSCRIPTEN__
	float renderDistance = 320.f;
#elif __HAIKU__
	float renderDistance = 256.f;
#else
	float renderDistance = 384.f;
#endif
float fadeoutDistance      =  32.f;
float fadeoutStartDistance = 192;
float cloudFadeD           = 256*256;
float cloudMinD            = 256*256*3;
float cloudMaxD            = 256*256*4;
float renderDistanceSquare = 256*256;

static void recalcDistances(){
	renderDistance = MIN(renderDistance,512.f);
	renderDistanceSquare = renderDistance * renderDistance;
	fadeoutDistance = renderDistance / 8.f;
	fadeoutStartDistance = renderDistance - fadeoutDistance;
	cloudFadeD = ((renderDistance/2) * (renderDistance/2));
	cloudMinD = cloudFadeD*3;
	cloudMaxD = cloudFadeD+cloudMinD;
}

void setRenderDistance(float newRD){
	renderDistance = newRD;
	recalcDistances();
}

GLenum glCheckError_(const char *file, int line){
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR){
		const char *error = "Unknown";
		switch (errorCode){
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM";                  break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE";                 break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION";             break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY";                 break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		fprintf(stderr,"glError: (%i) %s in (%s:%i)\n",errorCode,error,file,line);
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void initGL(){
	INITGLEXT();
	recalcDistances();
	glClearColor( 0.32f, 0.63f, 0.96f, 1.f );
	glViewport(0,0,screenWidth,screenHeight);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc( GL_LEQUAL );

	glScissor(0,0,screenWidth,screenHeight);
	glEnable(GL_SCISSOR_TEST);

#ifndef __EMSCRIPTEN__
	glEnable(GL_PROGRAM_POINT_SIZE);
#endif

#ifndef WOLKENWELTEN__GL_ES
	if(optionWireframe){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}else{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
#endif
}

void calcFOV(const character *cam){
	float off = vecMag(cam->vel);

	if(off < 0.025f){
		off = 0.f;
	}else{
		off = (off - 0.025f)*40.f;
	}
	off += 80.0f;
	if(off > gfxCurFOV){
		gfxCurFOV += (off-gfxCurFOV)/8;
	}else if(off < gfxCurFOV){
		gfxCurFOV -= (gfxCurFOV-off)/8;
	}
	if(gfxCurFOV <  80.1){gfxCurFOV =  80.0f;}
	if(gfxCurFOV > 170.1){gfxCurFOV = 170.0f;}
	float fov = gfxCurFOV / (1.f + (cam->zoomFactor * cam->aimFade));
	matPerspective(matProjection, fov, (float)screenWidth / (float)screenHeight, 0.165f, renderDistance + 32.f);
}

vec calcShake(const character *cam){
	static u64 ticks=0;
	if(cam->shake < 0.001f){return vecZero();}
	float deg = ((float)++ticks)*0.4f;
	const float   yaw = sin(deg*1.3f)*(cam->shake/2.f);
	const float pitch = cos(deg*2.1f)*(cam->shake/2.f);
	return vecNew(yaw,pitch,0.f);
}

void calcView(const character *cam){
	matIdentity(matView);
	camShake = calcShake(cam);
	const vec shake = vecAdd(cam->rot,camShake);
	matMulRotXY(matView,shake.yaw,shake.pitch);
	if(optionThirdPerson){
		vec cpos = vecAdd(cam->pos,vecNew(0,0.5f,0));
		cpos = vecSub(cpos,vecDegToVec(shake));
		matMulTrans(matView,-cpos.x,-cpos.y,-cpos.z);
	}else{
		matMulTrans(matView,-cam->pos.x,-(cam->pos.y+0.5+cam->yoff),-cam->pos.z);
	}
}

void renderWorld(const character *cam){
	worldDraw(cam);
	blockMiningDraw();
	animalDrawAll();
	entityDrawAll();
	characterDrawAll();
	cloudsRender();
	rainDrawAll();

	projectileDrawAll();
	fireDrawAll();
	waterDrawAll();
	particleDraw();

	shadowDraw();
	ropeDrawAll();
}

void renderFrame(){
	chunkResetCounter();
	calcFOV(player);
	calcView(player);
	cloudsCalcColors();

	renderSky(player);
	renderWorld(player);
	glClear(GL_DEPTH_BUFFER_BIT);
	renderUI();
	swapWindow();
	fpsTick();
}
