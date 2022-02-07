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

#include "../gfx/gfx.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/light.h"
#include "../game/projectile.h"
#include "../game/rope.h"
#include "../game/weather/weather.h"
#include "../gfx/boundaries.h"
#include "../gfx/fluid.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/sky.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../misc/options.h"
#include "../sdl/sdl.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../voxel/meshgen/shared.h"
#include "../../../common/src/game/hook.h"
#include "../../../common/src/misc/bmp.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/colors.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

bool  queueScreenshot = false;

#ifdef __EMSCRIPTEN__
bool  gfxUseSubData = true;
#else
bool  gfxUseSubData = false;
#endif

float matProjection[16], matSubBlockView[16], matView[16], matSkyProjection[16];

vec    subBlockViewOffset;
int    screenWidth          = 800;
int    screenHeight         = 600;
int    screenRefreshRate    = 60;
uint   frameRelaxedDeadline = 0;
size_t vboTrisCount         = 0;
size_t drawCallCount        = 0;
float  gfxCurFOV            = 90.0f;
vec    camShake;


#if defined(__HAIKU__) || defined(__EMSCRIPTEN__) || defined(__aarch64__) || defined(__ARM_ARCH_7A__)
	float renderDistance = 192.f;
#else
	float renderDistance = 384.f;
#endif

float fadeoutDistance      =  32.f;
float fadeoutStartDistance = 192;
float cloudFadeD           = 256*256;
float cloudMinD            = 256*256*3;
float cloudMaxD            = 256*256*4;
float renderDistanceSquare = 256*256;
bool gfxInitComplete = false;

/* Recalculate all valued dependent on the renderDistance, This is mostly
 * done for performance reasons, although I haven't tested this and the benefits might be zilch.
 */
static void recalcDistances(){
	renderDistance = MIN(renderDistance,512.f);
	renderDistanceSquare = renderDistance * renderDistance;
	fadeoutDistance = renderDistance / 8.f;
	fadeoutStartDistance = renderDistance - fadeoutDistance;
	cloudFadeD = ((renderDistance/2) * (renderDistance/2));
	cloudMinD = cloudFadeD*3;
	cloudMaxD = cloudFadeD+cloudMinD;
}

/* Set a new Render Distance and recalculate all the dependent values */
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

/* (re)initialize some OpenGL Values, can be called multiple times, and for the most part
 * shouldn't have much of an effect, definetly has to be called though when the Viewport Dimensions
 * have changed
 */
void initGL(){
	if(!glInitialize()){
		exit(3);
	}
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

	gfxInitComplete = true;

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

/* Calculate FOV based on a characters velocity as well as recalculate the Projection Matrices.
 * Should only be called once when starting to render a new frame. */
void calcFOV(const character *cam){
	if(cam == NULL){return;}
	float off = vecMag(cam->vel);

	if(off < 0.025f){
		off = 0.f;
	}else{
		off = (off - 0.025f)*40.f;
	}
	off += 90.0f;
	if(off > gfxCurFOV){
		gfxCurFOV += (off-gfxCurFOV)/8;
	}else if(off < gfxCurFOV){
		gfxCurFOV -= (gfxCurFOV-off)/8;
	}
	gfxCurFOV = MINMAX(gfxCurFOV,90.f,170.f);
	float fov = gfxCurFOV / cam->zoomFactor;
	matPerspective(matProjection,    fov, (float)screenWidth / (float)screenHeight, 0.165f, renderDistance + 32.f);
	matPerspective(matSkyProjection, fov, (float)screenWidth / (float)screenHeight, 1.f, 4096.f);
}

/* Return a rotation vector based on a characters shake value */
vec calcShake(const character *cam){
	if(cam == NULL){return vecZero();}
	static u64 ticks=0;
	if(cam->shake < 0.001f){return vecZero();}
	float deg = ((float)++ticks)*0.4f;
	const float   yaw = sinf(deg*1.3f)*(cam->shake/2.f);
	const float pitch = cosf(deg*2.1f)*(cam->shake/2.f);
	return vecNew(yaw,pitch,0.f);
}

/* Calculate the matView and matSubBlockView matrices, should only be called
 * once at the beginning of rendering a Frame */
void calcView(const character *cam){
	if(cam == NULL){return;}
	matIdentity(matView);
	matIdentity(matSubBlockView);
	camShake = calcShake(cam);
	const vec shake = vecAdd(cam->rot,camShake);
	matMulRotXY(matView,shake.yaw,shake.pitch);
	matMulRotXY(matSubBlockView,shake.yaw,shake.pitch);
	if(optionThirdPerson){
		vec cpos = vecAdd(cam->pos,vecNew(0,0.5f,0));
		cpos = vecSub(cpos,vecDegToVec(shake));
		matMulTrans(matView,-cpos.x,-cpos.y,-cpos.z);
		subBlockViewOffset = vecNew((int)cpos.x,(int)cpos.y,(int)cpos.z);
		matMulTrans(matSubBlockView,subBlockViewOffset.x-cpos.x,subBlockViewOffset.y-cpos.y,subBlockViewOffset.z-cpos.z);
	}else{
		matMulTrans(matView,-cam->pos.x,-(cam->pos.y+0.5+cam->yoff),-cam->pos.z);
		subBlockViewOffset = vecNew((int)cam->pos.x,(int)(cam->pos.y+0.5+cam->yoff),(int)cam->pos.z);
		matMulTrans(matSubBlockView,subBlockViewOffset.x-cam->pos.x,subBlockViewOffset.y-(cam->pos.y+0.5+cam->yoff),subBlockViewOffset.z-cam->pos.z);
	}
}

/* Render the World, as seen by character cam. */
void renderWorld(const character *cam){
	gfxGroupStart("World");
	worldDraw(cam);
	blockMiningDraw();
	animalDrawAll();
	entityDrawAll();
	characterDrawAll();
	characterDrawConsHighlight(cam);
	cloudsRender();
	snowDrawAll();
	rainDrawAll();
	shadowDraw();

	projectileDrawAll();
	fireDrawAll();
	fluidGenerateParticles();
	particleDraw();

	ropeDrawAll();
	gfxGroupEnd();
}

/* Actually take a screenshot, should only be called when queueScreenshot is true */
static void doScreenshot(){
	const uint len = screenWidth * screenHeight * 4;
	u32 *pixels = malloc(len);
	glReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	saveBMP("screenshot.bmp", screenWidth, screenHeight, pixels);
	queueScreenshot = false;
	free(pixels);
}

/* Render a single Frame */
void renderFrame(){
	chunkResetCounter();
	lightCheckTime();
	const int rRate = screenRefreshRate ? screenRefreshRate : 60; // Default to 60
	frameRelaxedDeadline = getTicks() + ((1000/rRate)/4); // Gotta hurry after 1/4 frame
	calcFOV(player);
	calcView(player);
	cloudsCalcColors();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(gameRunning){
		renderSky(player);
		if(drawBoundariesStyle){
			drawBoundaries(drawBoundariesStyle == 2 ? CHUNGUS_SIZE : CHUNK_SIZE);
		}
		renderWorld(player);
		glClear(GL_DEPTH_BUFFER_BIT);
	}else{
		drawMenuBackground();
	}

	renderUI();
	swapWindow();
	if(queueScreenshot){doScreenshot();}
	fpsTick();
}
