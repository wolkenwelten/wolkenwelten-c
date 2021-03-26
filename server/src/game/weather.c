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

#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/rain.h"

void weatherDoRain(){
	const ivec toff = ivecNewV(vecFloor(cloudOff));
	for(uint i=0;i<chungusCount;i++){
		if(rngValA(255) > rainIntensity){continue;}
		const chungus *c = &chungusList[i];
		if(c->y & 1){continue;}
		const vec cpos = vecNew(c->x << 8, c->y << 8, c->z << 8);
		u8 x = rngValA(255);
		u8 z = rngValA(255);
		const int tx = (x-toff.x) & 0xFF;
		const int tz = (z-toff.z) & 0xFF;
		int v = cloudTex[tx][tz];
		if(v > (cloudDensityMin+16)){continue;}
		const vec rpos = vecAdd(cpos,vecNew(x,32.f,z));
		rainNew(rpos);
	}
}
