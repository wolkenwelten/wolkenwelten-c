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
#include "../game/projectile.h"
#include "../game/rain.h"
#include "../game/rope.h"
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
#include "../../../common/src/misc/misc.h"

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


#if   defined(__HAIKU__)
	float renderDistance = 192.f;
#elif defined(__EMSCRIPTEN__) || defined(__aarch64__) || defined(__ARM_ARCH_7A__)
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
bool gfxInitComplete = false;

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
	if(!glInitialize()){exit(3);}
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

void calcFOV(const character *cam){
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
	if(gfxCurFOV <  90.1){gfxCurFOV =  90.0f;}
	if(gfxCurFOV > 170.1){gfxCurFOV = 170.0f;}
	float fov = gfxCurFOV / (1.f + (cam->zoomFactor * cam->aimFade));
	matPerspective(matProjection,    fov, (float)screenWidth / (float)screenHeight, 0.165f, renderDistance + 32.f);
	matPerspective(matSkyProjection, fov, (float)screenWidth / (float)screenHeight, 1.f, 4096.f);
}

vec calcShake(const character *cam){
	static u64 ticks=0;
	if(cam->shake < 0.001f){return vecZero();}
	float deg = ((float)++ticks)*0.4f;
	const float   yaw = sinf(deg*1.3f)*(cam->shake/2.f);
	const float pitch = cosf(deg*2.1f)*(cam->shake/2.f);
	return vecNew(yaw,pitch,0.f);
}

void calcView(const character *cam){
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

void renderWorld(const character *cam){
	(void)cam;
	worldDraw(cam);
	blockMiningDraw();
	animalDrawAll();
	entityDrawAll();
	characterDrawAll();
	characterDrawConsHighlight(cam);
	cloudsRender();
	rainDrawAll();

	projectileDrawAll();
	fireDrawAll();
	particleDraw();

	shadowDraw();
	ropeDrawAll();
}


#pragma pack(push, 1)
typedef struct {
	u8  idlength;
	u8  colourmaptype;
	u8  datatypecode;
	u16 colourmaporigin;
	u16 colourmaplength;
	u8  colourmapdepth;
	u16 x_origin;
	u16 y_origin;
	u16 width;
	u16 height;
	u8  bitsperpixel;
	u8  imagedescriptor;
} tgaHeader;
#pragma pack(pop)

static void doScreenshot(){
	const uint len = screenWidth * screenHeight * 3;
	void *pixels = calloc(1, len + sizeof(tgaHeader));
	tgaHeader *th = (tgaHeader *)pixels;
	th->datatypecode = 2; // Uncompressed RGB
	th->bitsperpixel = 24;
	th->width = screenWidth;
	th->height = screenHeight;

	glReadPixels(0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels + sizeof(tgaHeader));
	saveFile("screenshot.tga", pixels, len);
	queueScreenshot = false;
}

void renderFrame(){
	chunkResetCounter();
	frameRelaxedDeadline = getTicks() + ((1000/screenRefreshRate)/4); // Gotta hurry after 1/4 frame
	calcFOV(player);
	calcView(player);
	cloudsCalcColors();

	if(gameRunning){
		renderSky(player);
		renderWorld(player);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	renderUI();
	swapWindow();
	if(queueScreenshot){doScreenshot();}
	fpsTick();
}
