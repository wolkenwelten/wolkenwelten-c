#include "vec.h"
#include "../common.h"

#include <math.h>
#include <string.h>

float vecMag(const vec a){
	float dot = vecDot(a,a);
	return (dot > 0) ? sqrtf(dot) : 0;
}
vec vecFloor(const vec a){
	return (vec){{{floorf(a.x),floorf(a.y),floorf(a.z)}}};
}
vec vecSqrt(const vec a){
	return (vec){{{sqrtf(a.x),sqrtf(a.y),sqrtf(a.z)}}};
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
	ret.x =   atan2f( a.z,a.x)*180/PI + 90.f;
	ret.y =   atan2f(-a.y,a.z)*180/PI;
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
