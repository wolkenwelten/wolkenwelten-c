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

#include "../../asm/asm.h"
#include "../../game/weather/weather.h"
#include "../../misc/profiling.h"
#include "../../network/packet.h"
#include "../../network/messages.h"

void snowTick();
void rainTick();

vec windVel,windGVel;

void weatherInit(){
	cloudsInit();
	rainInit();
	snowInit();
	stormInit();
	windInit();
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
		if(!isClient){rainTick();}
		if((calls & 0x1FFF) == 1){
			if(cloudDensityMin < 160){
				rainIntensity++;
			}else{
				rainIntensity--;
			}
		} else if((calls & 0xFFF) == 2){
			if(rngValA(255) < rainIntensity){
				cloudGDensityMin++;
				if(!isClient){weatherSendUpdate(-1);}
			}
		}else if(((calls & 0x3FF) == 3) && (cloudDensityMin > 180)){
			rainIntensity--;
		}
	}
	if(snowIntensity){
		if(!isClient){snowTick();}
		if((calls & 0x1FF) == 1){
			if(cloudDensityMin < 160){
				snowIntensity++;
			}else{
				snowIntensity--;
			}
		} else if((calls & 0x3FFF) == 2){
			if(rngValA(255) < snowIntensity){
				cloudGDensityMin++;
				if(!isClient){weatherSendUpdate(-1);}
			}
		}else if(((calls & 0x3FFF) == 3) && (cloudDensityMin > 180)){
			snowIntensity--;
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

	rainUpdateAll();
	snowUpdateAll();
	stormUpdate();
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
	p->v.u8 [39] = snowIntensity;

	p->v.u8 [40] = stormIntensity;
	p->v.u8 [41] = stormDelta;
	p->v.u8 [42] = 0;
	p->v.u8 [43] = 0;

	packetQueue(p,msgtWeatherRecvUpdate,11*4,c);
}

void weatherRecvUpdate(const packet *p){
	cloudOff = vecNewP(&p->v.f[0]);
	windVel  = vecNewP(&p->v.f[3]);
	windGVel = vecNewP(&p->v.f[6]);

	cloudDensityMin   = p->v.u8[36];
	cloudGDensityMin  = p->v.u8[37];
	rainIntensity     = p->v.u8[38];
	snowIntensity     = p->v.u8[39];

	stormIntensity    = p->v.u8[40];
	stormDelta        = p->v.u8[41];
}
