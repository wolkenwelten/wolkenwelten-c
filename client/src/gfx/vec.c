#include "vec.h"

#include <math.h>
#include <string.h>

vec vecNew (float x, float y, float z){
	vec ret = {{{ x,y,z }}};
	return ret;
}
vec vecZero(){
	vec ret = {{{0.f,0.f,0.f}}};
	return ret;
}

vec   vecAdd (const vec a, const vec b){
	vec ret = {{{a.x+b.x,a.y+b.y,a.z+b.z}}};
	return ret;
}
vec   vecAddS(const vec a, const float b){
	vec ret = {{{a.x+b,a.y+b,a.z+b}}};
	return ret;
}

vec   vecSub (const vec a, const vec b){
	vec ret = {{{a.x-b.x,a.y-b.y,a.z-b.z}}};
	return ret;	
}
vec   vecSubS(const vec a, const float b){
	vec ret = {{{a.x-b,a.y-b,a.z-b}}};
	return ret;	
}

vec   vecMul (const vec a, const vec b){
	vec ret = {{{a.x*b.x,a.y*b.y,a.z*b.z}}};
	return ret;		
}
vec   vecMulS(const vec a, const float b){
	vec ret = {{{a.x*b,a.y*b,a.z*b}}};
	return ret;		
}

vec   vecDiv (const vec a, const vec b){
	vec ret = {{{a.x/b.x,a.y/b.y,a.z/b.z}}};
	return ret;		
}
vec   vecDivS(const vec a, const float b){
	vec ret = {{{a.x/b,a.y/b,a.z/b}}};
	return ret;		
}

float vecDot (const vec a, const vec b){
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

float vecMag (const vec a){
	float dot = vecDot(a,a);
	if(dot > 0){
		return sqrtf(dot);
	}else{
		return 0;
	}
}

vec   vecNorm(const vec a){
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

vec   vecSqrt(const vec a){
	vec ret = {{{sqrtf(a.x),sqrtf(a.y),sqrtf(a.z)}}};
	return ret;
}

vec   vecCross(const vec a, const vec b){
	vec ret;
	ret.x =   a.y * b.z - a.z * b.y;
	ret.y = -(a.x * b.z - a.z * b.x);
	ret.z =   a.x * b.y - a.y * b.x;	
	return ret;
}

vec  vecRotate(const vec a, const vec b, const float rad){
	float cos_theta = cosf(rad);
	float sin_theta = sinf(rad);
	vec ret = vecMulS(vecMulS(b,vecDot(a,b)),1-cos_theta);
	ret = vecAdd(ret,vecMulS(vecCross(b,a),sin_theta));
	ret = vecAdd(ret,vecMulS(a,cos_theta));
	return ret;
}
