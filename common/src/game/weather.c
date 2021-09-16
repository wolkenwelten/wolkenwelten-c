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

#include "weather.h"

#include "../asm/asm.h"
#include "../game/fire.h"
#include "../game/weather.h"
#include "../misc/noise.h"
#include "../misc/profiling.h"
#include "../network/packet.h"
#include "../network/messages.h"
#include "../world/world.h"

u8  cloudTex[256][256];
vec cloudOff;
vec windVel,windGVel;
u8  cloudGDensityMin;
u8  cloudDensityMin;
u8  rainIntensity;

__attribute__((aligned(32))) glRainDrop glRainDrops[RAIN_MAX+4];
__attribute__((aligned(32)))   rainDrop   rainDrops[RAIN_MAX+4];
__attribute__((aligned(32)))      float   rainVel[8];
                                    u64   rainCoords[RAIN_MAX+4];
uint rainCount = 0;

void rainPosUpdate();
void fxRainDrop(const vec pos);

void weatherInit(){
	generateNoise(0x84407db3, cloudTex);
	windGVel   = vecMulS(vecRng(),1.f/512.f);
	windGVel.y = 0.f;
	windVel    = windGVel;
	cloudGDensityMin  = 154;
	cloudDensityMin   = cloudGDensityMin;
	rainIntensity = 0;
}

void weatherUpdateAll(){
	static uint calls = 0;
	PROFILE_START();
	cloudDensityMin  = MAX(128,cloudDensityMin);
	cloudGDensityMin = MAX(128,cloudGDensityMin);

	if(!isClient && (rngValA((1<<18)-1) == 0)){
		windGVel   = vecMulS(vecRng(),1.f/512.f);
		windGVel.y = 0.f;
		weatherSendUpdate(-1);
	}
	if(!isClient && (rngValA((1<<16)-1) == 0)){
		--cloudGDensityMin;
		weatherSendUpdate(-1);
	}
	if(rainIntensity){
		if(!isClient){weatherDoRain();}
		if((calls & 0x1FF) == 1){
			if(cloudDensityMin < 160){
				rainIntensity++;
			}else{
				rainIntensity--;
			}
		} else if((calls & 0xFF) == 2){
			if(rngValA(255) < rainIntensity){
				cloudGDensityMin++;
				if(!isClient){weatherSendUpdate(-1);}
			}
		}else if(((calls & 0x3F) == 3) && (cloudDensityMin > 180)){
			rainIntensity--;
		}
	}
	if((!isClient) && (rainIntensity == 0) && (cloudDensityMin < 150) && (calls & 0xFFF) == 0){
		const uint chance = MAX(2,16 - (150 - cloudDensityMin));
		if(rngValA((1<<chance)-1) == 0){
			rainIntensity = 1;
			weatherSendUpdate(-1);
		}
	}
	if((cloudGDensityMin != cloudDensityMin) && ((calls & 0xFF) == 0)){
		if(cloudGDensityMin > cloudDensityMin){
			cloudDensityMin++;
		}else{
			cloudDensityMin--;
		}
	}
	windVel = vecMulS(vecAdd(vecMulS(windVel,255.f),windGVel),1.f/256.f);
	cloudOff = vecAdd(cloudOff,windVel);
	if(cloudOff.x > 256.f){cloudOff.x -= 256.f;}
	if(cloudOff.y > 256.f){cloudOff.y -= 256.f;}
	if(cloudOff.z > 256.f){cloudOff.z -= 256.f;}
	if(cloudOff.x <   0.f){cloudOff.x += 256.f;}
	if(cloudOff.y <   0.f){cloudOff.y += 256.f;}
	if(cloudOff.z <   0.f){cloudOff.z += 256.f;}

	calls++;
	PROFILE_STOP();
}

void weatherSendUpdate(uint c){
	packet *p = &packetBuffer;

	p->v.f  [0] = cloudOff.x;
	p->v.f  [1] = cloudOff.y;
	p->v.f  [2] = cloudOff.z;

	p->v.f  [3] = windVel.x;
	p->v.f  [4] = windVel.y;
	p->v.f  [5] = windVel.z;

	p->v.f  [6] = windGVel.x;
	p->v.f  [7] = windGVel.y;
	p->v.f  [8] = windGVel.z;

	p->v.u8 [36] = cloudDensityMin;
	p->v.u8 [37] = cloudGDensityMin;
	p->v.u8 [38] = rainIntensity;
	p->v.u8 [39] = 0;

	packetQueue(p,msgtWeatherRecvUpdate,10*4,c);
}

void weatherRecvUpdate(const packet *p){
	cloudOff = vecNewP(&p->v.f[0]);
	windVel  = vecNewP(&p->v.f[3]);
	windGVel = vecNewP(&p->v.f[6]);

	cloudDensityMin   = p->v.u8[36];
	cloudGDensityMin  = p->v.u8[37];
	rainIntensity = p->v.u8[38];
}

void cloudsSetWind(const vec ngv){
	windGVel = ngv;
	if(!isClient){weatherSendUpdate(-1);}
}

void cloudsSetDensity(u8 gd){
	cloudGDensityMin = gd;
	if(!isClient){weatherSendUpdate(-1);}
}

void weatherSetRainDuration(u8 dur){
	rainIntensity = dur;
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

void rainNew(vec pos){
	uint i = ++rainCount;
	if(i >= RAIN_MAX){i = rngValA(RAIN_MAX-1); rainCount--;}

	glRainDrops[i]  = (glRainDrop){ pos.x, pos.y, pos.z, 256.f };
	  rainDrops[i]  = (  rainDrop){ windVel.x, -0.1f, windVel.z, -0.1f };
          rainCoords[i] = 0;

	if(!isClient){rainSendUpdate(-1,i);}
}

static void rainDel(uint i){
	glRainDrops[i]  = glRainDrops[--rainCount];
	  rainDrops[i]  =   rainDrops[  rainCount];
	  rainCoords[i] =   rainCoords[ rainCount];
}

void rainPosUpdatePortable(){
	for(uint i=0;i<rainCount;i++){
		glRainDrops[i].x     += rainDrops[i].vx;
		glRainDrops[i].y     += rainDrops[i].vy;
		glRainDrops[i].z     += rainDrops[i].vz;
		glRainDrops[i].size  += rainDrops[i].vsize;

		  rainDrops[i].vx    += rainVel[0];
		  rainDrops[i].vy    += rainVel[1];
		  rainDrops[i].vz    += rainVel[2];
		  rainDrops[i].vsize += rainVel[3];
	}
}

void rainUpdateAll(){
	PROFILE_START();

	rainVel[0] = windVel.x / 40.f;
	rainVel[1] = -0.0005;
	rainVel[2] = windVel.z / 40.f;
	rainVel[3] = 0.f;

	for(uint i=0;i<4;i++){
		rainVel[i+4] = rainVel[i];
	}

	rainPosUpdate();
	for(uint i=rainCount-1;i<rainCount;i--){
		const glRainDrop *glrd = &glRainDrops[i];
		if((glrd->y < 0.f) || (glrd->size < 0.f)){
			rainDel(i);
			continue;
		}
		const u64 newCoords = ((u64)glrd->x & 0xFFFF) | (((u64)glrd->y & 0xFFFF) << 16) | (((u64)glrd->z & 0xFFFF) << 32);
		if(newCoords != rainCoords[i]){
			if(!worldIsLoaded(glrd->x,glrd->y,glrd->z)){
				rainDel(i);
				continue;
			}
			if(worldGetB(glrd->x,glrd->y,glrd->z) != 0){
				if(isClient){
					fxRainDrop(vecNew(glrd->x,glrd->y,glrd->z));
				}else{
					fireBoxExtinguish (glrd->x-1, glrd->y-1, glrd->z-1, 3, 3, 3, 256);
				}
				rainDel(i);
				continue;
			}
			rainCoords[i] = newCoords;
		}
	}

	PROFILE_STOP();
}

void rainSendUpdate(uint c, uint i){
	packet *p = &packetBuffer;

	p->v.f[0] = glRainDrops[i].x;
	p->v.f[1] = glRainDrops[i].y;
	p->v.f[2] = glRainDrops[i].z;

	packetQueue(p,msgtRainRecvUpdate,3*4,c);
}
