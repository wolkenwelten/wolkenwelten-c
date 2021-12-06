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
#include "storm.h"
#include "weather.h"

#include "../../misc/profiling.h"

u8 stormIntensity = 0;
i8 stormDelta = 0;

void stormInit(){
	stormIntensity = 0;
	stormDelta = 0;
}

void stormUpdate(){
	static uint calls = 0;
	PROFILE_START();


	if(((calls & 0x1FF) == 0)){
		if(stormDelta > 0){
			if(stormIntensity < 255){++stormIntensity;}
			if(rngValA((1<<12)-1) == 0){
				stormDelta = 0;
			}
		}else{
			if(stormIntensity > 0){
				--stormIntensity;
			}else if(rngValA((1<<16)-1) == 0){
				stormDelta = 1;
			}
		}
		if(!isClient){weatherSendUpdate(-1);}
	}
	if(((calls & 0x1FF) == 0)){
		if(!isClient && stormIntensity){
			if(rngValA(1<<20) < stormIntensity){
				windGVel   = vecMulS(vecRng(),1.f / (512-stormIntensity));
				windGVel.y = 0;
				weatherSendUpdate(-1);
			}
			if(rngValA(1<<14) < stormIntensity){
				windGVel = vecMulS(windGVel, (rngValf() * 0.3) + 0.9f);
				weatherSendUpdate(-1);
			}
		}
	}

	calls++;
	PROFILE_STOP();
}
