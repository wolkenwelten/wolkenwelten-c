#include "vec.h"
#include "misc.h"
#include "../common.h"

#include <math.h>
#include <string.h>

vec vecNew (float x, float y, float z){
	return (vec){{{ x,y,z }}};
}
vec vecNewP(const float *p){
	return (vec){{{p[0],p[1],p[2]}}};
}
vec vecNewU(const uvec a){
	return (vec){{{(float)a.x,(float)a.y,(float)a.z}}};
}
vec vecNewI(const ivec a){
	return (vec){{{(float)a.x,(float)a.y,(float)a.z}}};
}
vec vecZero(){
	return (vec){{{0.f,0.f,0.f}}};
}
vec vecOne(){
	return (vec){{{1.f,1.f,1.f}}};
}
vec vecRngAbs(){
	return (vec){{{rngValf(),rngValf(),rngValf()}}};
}
vec vecRng(){
	return vecMulS(vecSubS(vecRngAbs(),0.5f),2.f);
}
vec vecInvert(const vec a){
	return (vec){{{-a.x,-a.y,-a.z}}};
}

vec vecAdd (const vec a, const vec b){
	return (vec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
vec vecAddS(const vec a, const float b){
	return (vec){{{a.x+b,a.y+b,a.z+b}}};
}

vec vecSub (const vec a, const vec b){
	return (vec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
vec vecSubS(const vec a, const float b){
	return (vec){{{a.x-b,a.y-b,a.z-b}}};
}

vec vecMul (const vec a, const vec b){
	return (vec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
vec vecMulS(const vec a, const float b){
	return (vec){{{a.x*b,a.y*b,a.z*b}}};
}

vec vecDiv (const vec a, const vec b){
	return (vec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
vec vecDivS(const vec a, const float b){
	return (vec){{{a.x/b,a.y/b,a.z/b}}};
}

float vecDot (const vec a, const vec b){
	return (a.x*b.x)+(a.y*b.y)+(a.z*b.z);
}

float vecMag (const vec a){
	float dot = vecDot(a,a);
	if(dot > 0){
		return sqrtf(dot);
	}
	return 0;
}

vec vecFloor(const vec a){
	return (vec){{{floorf(a.x),floorf(a.y),floorf(a.z)}}};
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

vec vecSqrt(const vec a){
	return (vec){{{sqrtf(a.x),sqrtf(a.y),sqrtf(a.z)}}};
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

//TODO: y/pitch is not exact
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


ivec ivecNew(int x, int y, int z){
	return (ivec){{{x,y,z}}};
}
ivec ivecNewP(const int *p){
	return (ivec){{{p[0],p[1],p[2]}}};
}
ivec ivecZero(){
	return (ivec){{{0,0,0}}};
}
ivec ivecOne(){
	return (ivec){{{1,1,1}}};
}
ivec ivecNOne(){
	return (ivec){{{-1,-1,-1}}};
}
ivec ivecRng(){
	return (ivec){{{(int)rngValR(),(int)rngValR(),(int)rngValR()}}};
}
ivec ivecInvert(const ivec a){
	return (ivec){{{-a.x,-a.y,-a.z}}};
}
ivec ivecUvec(const uvec a){
	return (ivec){{{(int)a.x,(int)a.y,(int)a.z}}};
}

ivec ivecAdd(const ivec a, const ivec b){
	return (ivec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
ivec ivecAddS(const ivec a, const int  b){
	return (ivec){{{a.x+b  ,a.y+b  ,a.z+b  }}};
}
ivec ivecSub(const ivec a, const ivec b){
	return (ivec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
ivec ivecSubS(const ivec a, const int  b){
	return (ivec){{{a.x-b  ,a.y-b  ,a.z-b  }}};
}
ivec ivecMul(const ivec a, const ivec b){
	return (ivec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
ivec ivecMulS(const ivec a, const int  b){
	return (ivec){{{a.x*b  ,a.y*b  ,a.z*b  }}};
}
ivec ivecDiv(const ivec a, const ivec b){
	return (ivec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
ivec ivecDivS(const ivec a, const int  b){
	return (ivec){{{a.x/b  ,a.y/b  ,a.z/b  }}};
}
ivec ivecShlS(const ivec a, const int  b){
	return (ivec){{{a.x<<b ,a.y<<b ,a.z<<b }}};
}
ivec ivecShrS(const ivec a, const int  b){
	return (ivec){{{a.x>>b ,a.y>>b ,a.z>>b }}};
}
ivec ivecAnd(const ivec a, const ivec b){
	return (ivec){{{a.x&b.x,a.y&b.y,a.z&b.z}}};
}
ivec ivecAndS(const ivec a, const int b){
	return (ivec){{{a.x&b  ,a.y&b  ,a.z&b  }}};
}
ivec ivecOr(const ivec a, const ivec b){
	return (ivec){{{a.x|b.x,a.y|b.y,a.z|b.z}}};
}
ivec ivecOrS(const ivec a, const int b){
	return (ivec){{{a.x|b  ,a.y|b  ,a.z|b  }}};
}
int  ivecOrSum(const ivec a){
	return a.x|a.y|a.z;
}
ivec ivecXor(const ivec a, const ivec b){
	return (ivec){{{a.x^b.x,a.y^b.y,a.z^b.z}}};
}
ivec ivecXorS(const ivec a, const int b){
	return (ivec){{{a.x^b  ,a.y^b  ,a.z^b  }}};
}



uvec uvecNew(uint x, uint y, uint z){
	return (uvec){{{x,y,z}}};
}
uvec uvecNewP(const uint *p){
	return (uvec){{{p[0],p[1],p[2]}}};
}
uvec uvecZero(){
	return (uvec){{{0,0,0}}};
}
uvec uvecOne(){
	return (uvec){{{1,1,1}}};
}
uvec uvecRng(){
	return (uvec){{{rngValR(),rngValR(),rngValR()}}};
}
uvec uvecNot(const uvec a){
	return (uvec){{{~a.x,~a.y,~a.z}}};
}
uvec uvecIvec(const ivec a){
	return (uvec){{{(uint)a.x,(uint)a.y,(uint)a.z}}};
}

uvec uvecAdd(const uvec a, const uvec b){
	return (uvec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
uvec uvecAddS(const uvec a, const uint  b){
	return (uvec){{{a.x+b  ,a.y+b  ,a.z+b  }}};
}
uvec uvecSub(const uvec a, const uvec b){
	return (uvec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
uvec uvecSubS(const uvec a, const uint  b){
	return (uvec){{{a.x-b  ,a.y-b  ,a.z-b  }}};
}
uvec uvecMul(const uvec a, const uvec b){
	return (uvec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
uvec uvecMulS(const uvec a, const uint  b){
	return (uvec){{{a.x*b  ,a.y*b  ,a.z*b  }}};
}
uvec uvecDiv(const uvec a, const uvec b){
	return (uvec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
uvec uvecDivS(const uvec a, const uint  b){
	return (uvec){{{a.x/b  ,a.y/b  ,a.z/b  }}};
}
uvec uvecShlS(const uvec a, const uint  b){
	return (uvec){{{a.x<<b ,a.y<<b ,a.z<<b }}};
}
uvec uvecShrS(const uvec a, const uint  b){
	return (uvec){{{a.x>>b ,a.y>>b ,a.z>>b }}};
}
uvec uvecAnd(const uvec a, const uvec b){
	return (uvec){{{a.x&b.x,a.y&b.y,a.z&b.z}}};
}
uvec uvecAndS(const uvec a, const uint b){
	return (uvec){{{a.x&b  ,a.y&b  ,a.z&b  }}};
}
uvec uvecOr(const uvec a, const uvec b){
	return (uvec){{{a.x|b.x,a.y|b.y,a.z|b.z}}};
}
uvec uvecOrS(const uvec a, const uint b){
	return (uvec){{{a.x|b  ,a.y|b  ,a.z|b  }}};
}
uint uvecOrSum(const uvec a){
	return a.x|a.y|a.z;
}
uvec uvecXor(const uvec a, const uvec b){
	return (uvec){{{a.x^b.x,a.y^b.y,a.z^b.z}}};
}
uvec uvecXorS(const uvec a, const uint b){
	return (uvec){{{a.x^b  ,a.y^b  ,a.z^b  }}};
}
