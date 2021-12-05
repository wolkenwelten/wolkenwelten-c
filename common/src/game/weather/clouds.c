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
#include "clouds.h"
#include "weather.h"

#include "../../misc/noise.h"

u8  cloudTex[256][256];
vec cloudOff;
u8  cloudGDensityMin;
u8  cloudDensityMin;

void cloudsInit(){
	generateNoise(0x84407db3, cloudTex);
	cloudGDensityMin  = 154;
	cloudDensityMin   = cloudGDensityMin;
}

void cloudsSetDensity(u8 gd){
	cloudGDensityMin = gd;
	if(!isClient){weatherSendUpdate(-1);}
}

bool isInClouds(const vec p){
	const int ty = (uint)p.y >> 8;
	if(ty & 1){return false;}
	const int toffx = cloudOff.x;
	const int toffz = cloudOff.z;
	const int tx = ((u8)p.x-toffx) & 0xFF;
	const int tz = ((u8)p.z-toffz) & 0xFF;
	int v = cloudTex[tx][tz];
	if(v < (cloudDensityMin+2)){return false;}
	float cy = (ty << 8) + 32.0;
	float ymax = cy+(v-cloudDensityMin)*0.18;
	float ymin = cy-(v-cloudDensityMin)*0.09;
	return (p.y > ymin) && (p.y < ymax);
}
