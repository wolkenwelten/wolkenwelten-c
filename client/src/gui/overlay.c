#include "overlay.h"

#include "../sdl/sdl.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"

static uint32_t     overlayColorAnimSrc      = 0;
static uint32_t     overlayColorAnimDst      = 0;
static unsigned int overlayColorAnimStart    = 0;
static unsigned int overlayColorAnimEnd      = 1;
static uint32_t     overlayColorAnimNew      = 0;
static unsigned int overlayColorAnimDur      = 0;

uint32_t colorInterpolate(uint32_t c1,uint32_t c2,float i){
	if(i < 0.f){return c1;}
	if(i > 1.f){return c2;}

	const float r1 = (c1    ) & 0xFF;
	const float r2 = (c2    ) & 0xFF;
	const float g1 = (c1>> 8) & 0xFF;
	const float g2 = (c2>> 8) & 0xFF;
	const float b1 = (c1>>16) & 0xFF;
	const float b2 = (c2>>16) & 0xFF;
	const float a1 = (c1>>24) & 0xFF;
	const float a2 = (c2>>24) & 0xFF;
	const float        i2 = 1.f-i;

	const float r = (((float)r1 * i2) + ((float)r2 * i));
	const float g = (((float)g1 * i2) + ((float)g2 * i));
	const float b = (((float)b1 * i2) + ((float)b2 * i));
	const float a = (((float)a1 * i2) + ((float)a2 * i));

	return (uint32_t)r | ((uint32_t)g<<8) | ((uint32_t)b<<16) | ((uint32_t)a << 24);
}



void commitOverlayColor(){
	if(overlayColorAnimNew == overlayColorAnimDst){return;}
	overlayColorAnimStart = getTicks();
	overlayColorAnimEnd   = overlayColorAnimStart + overlayColorAnimDur;
	overlayColorAnimSrc   = overlayColorAnimDst;
	overlayColorAnimDst   = overlayColorAnimNew;
}

uint32_t getOverlayColor(){
	int animDur = overlayColorAnimEnd - overlayColorAnimStart;
	int off =  getTicks() - overlayColorAnimStart;
	float i = ((float)off) / ((float)animDur);
	return colorInterpolate(overlayColorAnimSrc,overlayColorAnimDst,i);
}

void setOverlayColor(unsigned int color, unsigned int animationDuration){
	if(animationDuration == 0){animationDuration = 1;}
	overlayColorAnimNew = color;
	overlayColorAnimDur = animationDuration;
}

void resetOverlayColor(){
	setOverlayColor(overlayColorAnimDst&0x000000,300);
}

void drawOverlay(textMesh *m){
	uint32_t c = getOverlayColor();
	if((c&0xFF000000) == 0){return;}
	textMeshBox(m, 0, 0, screenWidth, screenHeight, 19.f/32.f, 31.f/32.f, 1.f/32.f, 1.f/32.f, getOverlayColor());
}