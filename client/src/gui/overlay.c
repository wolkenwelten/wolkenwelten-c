#include "overlay.h"

#include "../sdl/sdl.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"
#include "../../../common/src/misc/misc.h"

static u32  nextOverlayColorAnimNew = 0;
static u32  nextOverlayColorAnimDur = 0;
static u32  overlayColorAnimSrc     = 0;
static u32  overlayColorAnimDst     = 0;
static uint overlayColorAnimStart   = 0;
static uint overlayColorAnimEnd     = 1;
static u32  overlayColorAnimNew     = 0;
static uint overlayColorAnimDur     = 0;

void commitOverlayColor(){
	if(overlayColorAnimNew == overlayColorAnimDst){return;}
	overlayColorAnimStart = getTicks();
	overlayColorAnimEnd   = overlayColorAnimStart + overlayColorAnimDur;
	overlayColorAnimSrc   = overlayColorAnimDst;
	overlayColorAnimDst   = overlayColorAnimNew;
}

u32 getOverlayColor(){
	int animDur = overlayColorAnimEnd - overlayColorAnimStart;
	int off =  getTicks() - overlayColorAnimStart;
	float i = ((float)off) / ((float)animDur);
	return colorInterpolate(overlayColorAnimSrc,overlayColorAnimDst,i);
}

void setOverlayColor(u32 color, u32 animationDuration){
	if(animationDuration == 0){animationDuration = 1;}
	overlayColorAnimNew = color;
	overlayColorAnimDur = animationDuration;
}

void nextOverlayColor(u32 color, u32 animationDuration){
	if(animationDuration == 0){animationDuration = 1;}
	nextOverlayColorAnimNew = color;
	nextOverlayColorAnimDur = animationDuration;
}

void resetOverlayColor(){
	setOverlayColor(overlayColorAnimDst&0x000000,300);
	if(nextOverlayColorAnimDur > 0){
		setOverlayColor(nextOverlayColorAnimNew,nextOverlayColorAnimDur);
		nextOverlayColorAnimDur = 0;
	}
}

void drawOverlay(textMesh *m){
	u32 c = getOverlayColor();
	if((c&0xFF000000) == 0){return;}
	textMeshBox(m, 0, 0, screenWidth, screenHeight, 19.f/32.f, 31.f/32.f, 1.f/32.f, 1.f/32.f, getOverlayColor());
}
