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
#include "rain.h"
#include "weather.h"

#include "../../game/fire.h"
#include "../../misc/profiling.h"
#include "../../world/world.h"

__attribute__((aligned(32)))   rainDrop glRainDrops[RAIN_MAX];
__attribute__((aligned(32)))   rainDrop   rainDrops[RAIN_MAX];
__attribute__((aligned(32)))      float   rainVel[8];
                                    u64   rainCoords[RAIN_MAX];
uint rainCount = 0;
u8 rainIntensity;

void rainInit(){
	rainIntensity = 0;
}

void rainPosUpdate();
void fxRainDrop(const vec pos);

void weatherSetRainIntensity(u8 intensity){
	rainIntensity = intensity;
}

void rainNew(vec pos){
	uint i = ++rainCount;
	if(i >= RAIN_MAX){i = rngValA(RAIN_MAX-1); rainCount--;}

	glRainDrops[i]  = (rainDrop){ pos.x, pos.y, pos.z, 256.f };
	  rainDrops[i]  = (rainDrop){ windVel.x, -0.1f, windVel.z, -0.1f };
          rainCoords[i] = 0;
}

static void rainDel(uint i){
	glRainDrops[i]  = glRainDrops[--rainCount];
	  rainDrops[i]  =   rainDrops[  rainCount];
	  rainCoords[i] =   rainCoords[ rainCount];
}

void rainPosUpdatePortable(){
	for(uint i=0;i<rainCount;i++){
		glRainDrops[i].x     += rainDrops[i].x;
		glRainDrops[i].y     += rainDrops[i].y;
		glRainDrops[i].z     += rainDrops[i].z;
		glRainDrops[i].size  += rainDrops[i].size;

		  rainDrops[i].x    += rainVel[0];
		  rainDrops[i].y    += rainVel[1];
		  rainDrops[i].z    += rainVel[2];
		  rainDrops[i].size += rainVel[3];
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
		const rainDrop *glrd = &glRainDrops[i];
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
					fireBoxExtinguish(glrd->x, glrd->y, glrd->z, 1, 1, 1, 255);
				}
				rainDel(i);
				continue;
			}
			rainCoords[i] = newCoords;
		}
	}

	PROFILE_STOP();
}
