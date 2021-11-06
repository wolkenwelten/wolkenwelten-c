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

#include "colors.h"

char *ansiRS = "\033[0m";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

static u32 colorInterpolateSingle(u32 c1, u32 c2, int shift, int v){
	uint acc = 0;
	acc += ((c1 >> shift) & 0xFF) * (256-v);
	acc += ((c2 >> shift) & 0xFF) *      v;
	acc = acc >> 8;
	return (acc & 0xFF) << shift;
}

u32 colorInterpolateRGB(u32 c1, u32 c2, float i){
	if(i < 0.01f){return c1;}
	if(i > 0.99f){return c2;}
	u32 ret = 0;
	const int v = (int)(i*255.f);
	ret |= colorInterpolateSingle(c1,c2, 0,v);
	ret |= colorInterpolateSingle(c1,c2, 8,v);
	ret |= colorInterpolateSingle(c1,c2,16,v);
	ret |= colorInterpolateSingle(c1,c2,24,v);
	return ret;
}


rgbaColor hsvToRGB(hsvaColor hsv){
	rgbaColor rgb;
	u8 region, remainder, p, q, t;

	if (hsv.s == 0){
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;

	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region){
	case 0:
		rgb.r = hsv.v; rgb.g = t; rgb.b = p;
		break;
	case 1:
		rgb.r = q; rgb.g = hsv.v; rgb.b = p;
		break;
	case 2:
		rgb.r = p; rgb.g = hsv.v; rgb.b = t;
		break;
	case 3:
		rgb.r = p; rgb.g = q; rgb.b = hsv.v;
		break;
	case 4:
		rgb.r = t; rgb.g = p; rgb.b = hsv.v;
		break;
	case 5:
	default:
		rgb.r = hsv.v; rgb.g = p; rgb.b = q;
		break;
	}

	return rgb;
}

hsvaColor rgbToHSV(rgbaColor rgb){
	hsvaColor hsv;
	u8 rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0){
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * ((int)rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0){
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r){
		hsv.h =   0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	} else if (rgbMax == rgb.g){
		hsv.h =  85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	} else {
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
	}

	return hsv;
}

u32 colorInterpolate(u32 c1, u32 c2, float i){
	const u32 cc1 = HSVAToU(rgbToHSV(uToRGBA(c1)));
	const u32 cc2 = HSVAToU(rgbToHSV(uToRGBA(c2)));
	const u32 cc  = colorInterpolateRGB(cc1,cc2,i);
	return RGBAToU(hsvToRGB(uToHSVA(cc)));
}
