#pragma once
#include "../stdint.h"

typedef struct {
	u8 a,b,g,r;
} rgbaColor;

typedef struct {
	u8 a,v,s,h;
} hsvaColor;

static inline rgbaColor uToRGBA(u32 rgba){
	return (rgbaColor){(rgba >> 24)&0xFF,(rgba >> 16)&0xFF,(rgba >> 8)&0xFF,rgba&0xFF};
}
static inline u32 RGBAToU(rgbaColor rgba){
	return rgba.r | (rgba.g << 8) | (rgba.b << 16) | (rgba.a << 24);
}
static inline hsvaColor uToHSVA(u32 hsva){
	return (hsvaColor){(hsva >> 24)&0xFF,(hsva >> 16)&0xFF,(hsva >> 8)&0xFF,hsva&0xFF};
}
static inline u32 HSVAToU(hsvaColor hsva){
	return hsva.h | (hsva.s << 8) | (hsva.v << 16) | (hsva.a << 24);
}

u32       colorInterpolateRGB (u32 c1, u32 c2, float i);
u32       colorInterpolate    (u32 c1, u32 c2, float i);
rgbaColor hsvToRGB            (hsvaColor hsv);
hsvaColor rgbToHSV            (rgbaColor rgb);
