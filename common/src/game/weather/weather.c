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

static void boundsCheckWindGoalVelocity(){
	const float curMin = 0.f + (stormIntensity / 25600.0f);
	const float curMax = 0.02f + (stormIntensity / 25600.0f);

	if(windGVel.x > 0){
		windGVel.x = MINMAX( curMin, curMax, windGVel.x);
	}else{
		windGVel.x = MINMAX(-curMax,-curMin, windGVel.x);
	}
	if(windGVel.y > 0){
		windGVel.y = MINMAX( curMin, curMax, windGVel.y);
	}else{
		windGVel.y = MINMAX(-curMax,-curMin, windGVel.y);
	}
	if(windGVel.z > 0){
		windGVel.z = MINMAX( curMin, curMax, windGVel.z);
	}else{
		windGVel.z = MINMAX(-curMax,-curMin, windGVel.z);
	}
}

void weatherUpdateAll(){
	static uint calls = 0;
	PROFILE_START();
	cloudDensityMin  = MAX(128,cloudDensityMin);
	cloudGDensityMin = MAX(128,cloudGDensityMin);

	if(!isClient && (rngValA((1<<16)-1) == 0)){
		windGVel   = vecMulS(vecRng(),1.f/256.f);
		windGVel.y = 0.f;
		boundsCheckWindGoalVelocity();
	}
	if(!isClient && (rngValA((1<<10)-1) == 0)){
		windGVel   = vecMulS(windGVel,rngValf() + 0.5f);
		windGVel.y = 0.f;
		boundsCheckWindGoalVelocity();
	}
	if(!isClient && (rngValA((1<<16)-1) == 0)){
		--cloudGDensityMin;
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
			}
		}else if(((calls & 0x3FFF) == 3) && (cloudDensityMin > 180)){
			snowIntensity--;
		}
	}
	if((!isClient) && (rainIntensity == 0) && (cloudDensityMin < 150) && (calls & 0xFFF) == 0){
		const uint chance = MAX(2,16 - (150 - cloudDensityMin));
		if(rngValA((1<<chance)-1) == 0){
			rainIntensity = 1;
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
	lightningDrawOverlay();
}
