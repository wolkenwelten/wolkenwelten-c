#pragma once
#include "rng.h"
#include "../gfx_structs.h"

vec vecSqrt     (const vec a);
vec vecCross    (const vec a, const vec b);
vec vecRotate   (const vec a, const vec b, const float rad);
vec vecNorm     (const vec a);
vec vecVecToDeg (const vec a);
vec vecDegToVec (const vec a);
vec vecCeil     (const vec a);
vec vecRound    (const vec a);

static inline vec vecNew (float x, float y, float z){
	return (vec){{{ x,y,z }}};
}
static inline vec vecNewP(const float *p){
	return (vec){{{p[0],p[1],p[2]}}};
}
static inline vec vecNewU(const uvec a){
	return (vec){{{a.x,a.y,a.z}}};
}
static inline vec vecNewI(const ivec a){
	return (vec){{{a.x,a.y,a.z}}};
}
static inline vec vecNOne(){
	return (vec){{{-1.f,-1.f,-1.f}}};
}
static inline vec vecZero(){
	return (vec){{{0.f,0.f,0.f}}};
}
static inline vec vecOne(){
	return (vec){{{1.f,1.f,1.f}}};
}
static inline vec vecInvert(const vec a){
	return (vec){{{-a.x,-a.y,-a.z}}};
}
static inline vec vecAdd (const vec a, const vec b){
	return (vec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
static inline vec vecAddS(const vec a, const float b){
	return (vec){{{a.x+b,a.y+b,a.z+b}}};
}
static inline vec vecAddT(const vec a, const vec b, const vec c){
	return (vec){{{a.x+b.x+c.x,a.y+b.y+c.y,a.z+b.z+c.z}}};
}
static inline vec vecSub (const vec a, const vec b){
	return (vec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
static inline vec vecSubS(const vec a, const float b){
	return (vec){{{a.x-b,a.y-b,a.z-b}}};
}
static inline vec vecMul (const vec a, const vec b){
	return (vec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
static inline vec vecMulS(const vec a, const float b){
	return (vec){{{a.x*b,a.y*b,a.z*b}}};
}
static inline vec vecMulT(const vec a, const vec b, const vec c){
	return (vec){{{a.x*b.x*c.x,a.y*b.y*c.y,a.z*b.z*c.z}}};
}
static inline vec vecDiv (const vec a, const vec b){
	return (vec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
static inline vec vecDivS(const vec a, const float b){
	return (vec){{{a.x/b,a.y/b,a.z/b}}};
}
static inline vec vecMod (const vec a, const vec b){
	return (vec){{{__builtin_fmodf(a.x,b.x),__builtin_fmodf(a.y,b.y),__builtin_fmodf(a.z,b.z)}}};
}
static inline vec vecAbs(const vec a){
	return (vec){{{__builtin_fabsf(a.x),__builtin_fabsf(a.y),__builtin_fabsf(a.z)}}};
}
static inline vec vecFloor(const vec a){
	return (vec){{{__builtin_floorf(a.x),__builtin_floorf(a.y),__builtin_floorf(a.z)}}};
}
static inline float vecDot (const vec a, const vec b){
	return (a.x*b.x)+(a.y*b.y)+(a.z*b.z);
}
static inline float vecMag(const vec a){
	float dot = vecDot(a,a);
	return (dot > 0) ? __builtin_sqrtf(dot) : 0;
}
static inline float vecSum(const vec a){
	return a.x+a.y+a.z;
}
static inline float vecAbsSum(const vec a){
	return __builtin_fabsf(a.x)+__builtin_fabsf(a.y)+__builtin_fabsf(a.z);
}
static inline vec vecRngAbs(){
	return (vec){{{rngValf(),rngValf(),rngValf()}}};
}
static inline vec vecRng(){
	return vecMulS(vecSubS(vecRngAbs(),0.5f),2.f);
}
static inline int vecInWorld(const vec a){
	if((a.x < 0.f) || (a.y < 0.f) || (a.z < 0.f))            {return 0;}
	if((a.x > 65536.f) || (a.y > 32768.f) || (a.z > 65536.f)){return 0;}
	return 1;
}

static inline ivec ivecNew(int x, int y, int z){
	return (ivec){{{x,y,z}}};
}
static inline ivec ivecNewV(const vec a){
	return (ivec){{{(int)a.x,(int)a.y,(int)a.z}}};
}
static inline ivec ivecNewP(const int *p){
	return (ivec){{{p[0],p[1],p[2]}}};
}
static inline ivec ivecZero(){
	return (ivec){{{0,0,0}}};
}
static inline ivec ivecOne(){
	return (ivec){{{1,1,1}}};
}
static inline ivec ivecNOne(){
	return (ivec){{{-1,-1,-1}}};
}
static inline ivec ivecRng(){
	return (ivec){{{(int)rngValR(),(int)rngValR(),(int)rngValR()}}};
}
static inline ivec ivecInvert(const ivec a){
	return (ivec){{{-a.x,-a.y,-a.z}}};
}
static inline ivec ivecUvec(const uvec a){
	return (ivec){{{(int)a.x,(int)a.y,(int)a.z}}};
}
static inline ivec ivecAdd(const ivec a, const ivec b){
	return (ivec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
static inline ivec ivecAddS(const ivec a, const int  b){
	return (ivec){{{a.x+b  ,a.y+b  ,a.z+b  }}};
}
static inline ivec ivecSub(const ivec a, const ivec b){
	return (ivec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
static inline ivec ivecSubS(const ivec a, const int  b){
	return (ivec){{{a.x-b  ,a.y-b  ,a.z-b  }}};
}
static inline ivec ivecMul(const ivec a, const ivec b){
	return (ivec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
static inline ivec ivecMulS(const ivec a, const int  b){
	return (ivec){{{a.x*b  ,a.y*b  ,a.z*b  }}};
}
static inline ivec ivecDiv(const ivec a, const ivec b){
	return (ivec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
static inline ivec ivecDivS(const ivec a, const int  b){
	return (ivec){{{a.x/b  ,a.y/b  ,a.z/b  }}};
}
static inline ivec ivecShlS(const ivec a, const int  b){
	return (ivec){{{a.x<<b ,a.y<<b ,a.z<<b }}};
}
static inline ivec ivecShrS(const ivec a, const int  b){
	return (ivec){{{a.x>>b ,a.y>>b ,a.z>>b }}};
}
static inline ivec ivecAnd(const ivec a, const ivec b){
	return (ivec){{{a.x&b.x,a.y&b.y,a.z&b.z}}};
}
static inline ivec ivecAndS(const ivec a, const int b){
	return (ivec){{{a.x&b  ,a.y&b  ,a.z&b  }}};
}
static inline ivec ivecOr(const ivec a, const ivec b){
	return (ivec){{{a.x|b.x,a.y|b.y,a.z|b.z}}};
}
static inline ivec ivecOrS(const ivec a, const int b){
	return (ivec){{{a.x|b  ,a.y|b  ,a.z|b  }}};
}
static inline ivec ivecXor(const ivec a, const ivec b){
	return (ivec){{{a.x^b.x,a.y^b.y,a.z^b.z}}};
}
static inline ivec ivecXorS(const ivec a, const int b){
	return (ivec){{{a.x^b  ,a.y^b  ,a.z^b  }}};
}
static inline int ivecOrSum(const ivec a){
	return a.x|a.y|a.z;
}
static inline int ivecSum  (const ivec a){
	return a.x+a.y+a.z;
}
static inline bool ivecEq  (const ivec a, const ivec b){
	return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}



static inline uvec uvecNew(uint x, uint y, uint z){
	return (uvec){{{x,y,z}}};
}
static inline uvec uvecNewV(const vec a){
	return (uvec){{{(uint)a.x,(uint)a.y,(uint)a.z}}};
}
static inline uvec uvecNewP(const uint *p){
	return (uvec){{{p[0],p[1],p[2]}}};
}
static inline uvec uvecZero(){
	return (uvec){{{0,0,0}}};
}
static inline uvec uvecOne(){
	return (uvec){{{1,1,1}}};
}
static inline uvec uvecRng(){
	return (uvec){{{rngValR(),rngValR(),rngValR()}}};
}
static inline uvec uvecNot(const uvec a){
	return (uvec){{{~a.x,~a.y,~a.z}}};
}
static inline uvec uvecIvec(const ivec a){
	return (uvec){{{(uint)a.x,(uint)a.y,(uint)a.z}}};
}
static inline uvec uvecAdd(const uvec a, const uvec b){
	return (uvec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
static inline uvec uvecAddS(const uvec a, const uint  b){
	return (uvec){{{a.x+b  ,a.y+b  ,a.z+b  }}};
}
static inline uvec uvecSub(const uvec a, const uvec b){
	return (uvec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
static inline uvec uvecSubS(const uvec a, const uint  b){
	return (uvec){{{a.x-b  ,a.y-b  ,a.z-b  }}};
}
static inline uvec uvecMul(const uvec a, const uvec b){
	return (uvec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
static inline uvec uvecMulS(const uvec a, const uint  b){
	return (uvec){{{a.x*b  ,a.y*b  ,a.z*b  }}};
}
static inline uvec uvecDiv(const uvec a, const uvec b){
	return (uvec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
static inline uvec uvecDivS(const uvec a, const uint  b){
	return (uvec){{{a.x/b  ,a.y/b  ,a.z/b  }}};
}
static inline uvec uvecShlS(const uvec a, const uint  b){
	return (uvec){{{a.x<<b ,a.y<<b ,a.z<<b }}};
}
static inline uvec uvecShrS(const uvec a, const uint  b){
	return (uvec){{{a.x>>b ,a.y>>b ,a.z>>b }}};
}
static inline uvec uvecAnd(const uvec a, const uvec b){
	return (uvec){{{a.x&b.x,a.y&b.y,a.z&b.z}}};
}
static inline uvec uvecAndS(const uvec a, const uint b){
	return (uvec){{{a.x&b  ,a.y&b  ,a.z&b  }}};
}
static inline uvec uvecOr(const uvec a, const uvec b){
	return (uvec){{{a.x|b.x,a.y|b.y,a.z|b.z}}};
}
static inline uvec uvecOrS(const uvec a, const uint b){
	return (uvec){{{a.x|b  ,a.y|b  ,a.z|b  }}};
}
static inline uint uvecOrSum(const uvec a){
	return a.x|a.y|a.z;
}
static inline uvec uvecXor(const uvec a, const uvec b){
	return (uvec){{{a.x^b.x,a.y^b.y,a.z^b.z}}};
}
static inline uvec uvecXorS(const uvec a, const uint b){
	return (uvec){{{a.x^b  ,a.y^b  ,a.z^b  }}};
}
static inline bool uvecEq  (const uvec a, const uvec b){
	return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}
