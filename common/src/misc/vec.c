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

#include "vec.h"
#include "../common.h"
#include "rng.h"

#include <math.h>


int vecInWorld(const vec a){
	if((a.x < 0.f) || (a.y < 0.f) || (a.z < 0.f))            {return 0;}
	if((a.x > 65536.f) || (a.y > 32768.f) || (a.z > 65536.f)){return 0;}
	return 1;
}

u64 vecToPacked (const vec v){
	const u64 x = v.x;
	const u64 y = v.y;
	const u64 z = v.z;
	return (x & 0xFFFF) | ((y & 0xFFFF) << 16) | ((z & 0xFFFF) << 32);
}

vec packedToVec (const u64 v){
	const u64 x =  v        & 0xFFFF;
	const u64 y = (v >> 16) & 0xFFFF;;
	const u64 z = (v >> 32) & 0xFFFF;;
	return vecNew(x,y,z);
}
