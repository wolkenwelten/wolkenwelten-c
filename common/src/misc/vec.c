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

#include <math.h>


vec vecSqrt(const vec a){
	return (vec){{{sqrtf(a.x),sqrtf(a.y),sqrtf(a.z)}}};
}
vec vecCeil(const vec a){
	return (vec){{{ceilf(a.x),ceilf(a.y),ceilf(a.z)}}};
}
vec vecRound(const vec a){
	return (vec){{{roundf(a.x),roundf(a.y),roundf(a.z)}}};
}

vec vecNorm(const vec a){
	vec ret = a;
	float mag = vecMag(a);
	if (mag > 0) {
		float invLen = 1 / mag;
		ret.x *= invLen;
		ret.y *= invLen;
		ret.z *= invLen;
	}
	return ret;
}

vec vecCross(const vec a, const vec b){
	vec ret;
	ret.x =   a.y * b.z - a.z * b.y;
	ret.y = -(a.x * b.z - a.z * b.x);
	ret.z =   a.x * b.y - a.y * b.x;
	return ret;
}

vec vecRotate(const vec a, const vec b, const float rad){
	float cos_theta = cosf(rad);
	float sin_theta = sinf(rad);
	vec ret = vecMulS(vecMulS(b,vecDot(a,b)),1-cos_theta);
	ret = vecAdd(ret,vecMulS(vecCross(b,a),sin_theta));
	ret = vecAdd(ret,vecMulS(a,cos_theta));
	return ret;
}

vec vecVecToDeg(const vec a){
	vec ret;
	ret.x = atan2f( a.z,a.x)*180/PI + 90.f;
	ret.y = atan2f(-a.y,a.z)*180/PI;
	ret.z = 0.f;
	return ret;
}

vec vecDegToVec(const vec a){
	vec ret;
	ret.x = cosf((a.x-90.f)*PI180) * cosf((-a.y)*PI180);
	ret.y = sinf((-a.y)*PI180);
	ret.z = sinf((a.x-90.f)*PI180) * cosf((-a.y)*PI180);
	return ret;
}
