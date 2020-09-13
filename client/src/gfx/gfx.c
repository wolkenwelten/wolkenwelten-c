#include "../gfx/gfx.h"

#include "../tmp/assets.h"
#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/grapplingHook.h"
#include "../gfx/mat.h"
#include "../gfx/shader.h"
#include "../gfx/sky.h"
#include "../gfx/particle.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../sdl/sdl.h"
#include "../voxel/chunk.h"
#include "../voxel/chungus.h"
#include "../voxel/bigchungus.h"


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../gfx/gl.h"

float matProjection[16], matView[16];

int screenWidth     = 800;
int screenHeight    = 600;
size_t vboTrisCount = 0;

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
	glClearColor( 0.00f, 0.05f, 0.2f, 1.f );
	glViewport(0,0,screenWidth,screenHeight);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc( GL_LEQUAL );

	glActiveTexture(GL_TEXTURE0);
#ifndef __EMSCRIPTEN__
	glDisable(GL_POLYGON_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_PROGRAM_POINT_SIZE);
#endif
}

double gfxCurFOV = 80.0;
void calcFOV(character *cam){
	float off = 0.f;

	off = sqrtf(cam->vx*cam->vx + cam->vy*cam->vy + cam->vz*cam->vz);

	if(off < 0.025f){
		off = 0.f;
	}else{
		off = (off - 0.025f)*50.f;
	}
	off += 80.0f;
	if(off > gfxCurFOV){
		gfxCurFOV += (off-gfxCurFOV)/8;
	}else if(off < gfxCurFOV){
		gfxCurFOV -= (gfxCurFOV-off)/8;
	}
	if(gfxCurFOV < 80.1){gfxCurFOV = 80.0;}
	matPerspective(matProjection, gfxCurFOV, (float)screenWidth / (float)screenHeight, 0.2f, 2560.0f);
}

void calcShake(character *cam, float *pitch, float *yaw){
	static uint64_t ticks=0;
	if(cam->shake < 0.001f){return;}
	float deg = ((float)++ticks)*0.4f;
	*pitch = cos(deg*2.1f)*(cam->shake/2.f);
	*yaw   = sin(deg*1.3f)*(cam->shake/2.f);
}

void calcView(character *cam){
	float shakeP=0.f,shakeY=0.f;
	matIdentity(matView);
	calcShake(cam,&shakeP,&shakeY);
	matMulRotXY(matView,cam->yaw+shakeP,cam->pitch+shakeY);
	matMulTrans(matView,-cam->x,-(cam->y+0.5+cam->yoff),-cam->z);
}

void renderWorld(character *cam){
	glDepthFunc( GL_LESS );
	bigchungusDraw(&world,cam);
	blockMiningDraw();
	grapplingHookDrawRopes();
	animalDrawAll();
	entityDrawAll();
	characterDrawAll();
	particleDraw();
	glDepthFunc( GL_LEQUAL );
}

void renderFrame(){
	chunkResetCounter();
	calcFOV(player);
	calcView(player);

	glClear(GL_DEPTH_BUFFER_BIT);
	renderSky(player);
	renderWorld(player);
	glClear(GL_DEPTH_BUFFER_BIT);
	renderUI();
	swapWindow();
	fpsTick();
	checkTexturesForReloading();
}

void renderMenuFrame(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderMenu();
	swapWindow();
	fpsTick();
	checkTexturesForReloading();
}
