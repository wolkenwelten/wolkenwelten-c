#pragma once
#include "../gfx_structs.h"

vec   vecNew     (float x, float y, float z);
vec   vecNewP    (const float *p);
vec   vecNewU    (const uvec a);
vec   vecNewI    (const ivec a);
vec   vecZero    ();
vec   vecOne     ();
vec   vecRng     ();
vec   vecRngAbs  ();
vec   vecInvert  (const vec a);

vec   vecAdd     (const vec a, const vec   b);
vec   vecAddS    (const vec a, const float b);
vec   vecSub     (const vec a, const vec   b);
vec   vecSubS    (const vec a, const float b);
vec   vecMul     (const vec a, const vec   b);
vec   vecMulS    (const vec a, const float b);
vec   vecDiv     (const vec a, const vec   b);
vec   vecDivS    (const vec a, const float b);

float vecDot     (const vec a, const vec   b);
float vecSum     (const vec a);
float vecMag     (const vec a);
vec   vecFloor   (const vec a);
vec   vecNorm    (const vec a);
vec   vecSqrt    (const vec a);
vec   vecCross   (const vec a, const vec   b);
vec   vecRotate  (const vec a, const vec   b, const float rad);
vec   vecVecToDeg(const vec a);
vec   vecDegToVec(const vec a);


ivec  ivecNew   (int x, int y, int z);
ivec  ivecNewV  (const vec a);
ivec  ivecNewP  (const int *p);
ivec  ivecZero  ();
ivec  ivecOne   ();
ivec  ivecNOne  ();
ivec  ivecRng   ();
ivec  ivecInvert(const ivec a);
ivec  ivecUvec  (const uvec a);

ivec  ivecAdd   (const ivec a, const ivec b);
ivec  ivecAddS  (const ivec a, const int  b);
ivec  ivecSub   (const ivec a, const ivec b);
ivec  ivecSubS  (const ivec a, const int  b);
ivec  ivecMul   (const ivec a, const ivec b);
ivec  ivecMulS  (const ivec a, const int  b);
ivec  ivecDiv   (const ivec a, const ivec b);
ivec  ivecDivS  (const ivec a, const int  b);
ivec  ivecShlS  (const ivec a, const int  b);
ivec  ivecShrS  (const ivec a, const int  b);
ivec  ivecAnd   (const ivec a, const ivec b);
ivec  ivecAndS  (const ivec a, const int  b);
ivec  ivecOr    (const ivec a, const ivec b);
ivec  ivecOrS   (const ivec a, const int  b);
ivec  ivecXor   (const ivec a, const ivec b);
ivec  ivecXorS  (const ivec a, const int  b);
int   ivecOrSum (const ivec a);
int   ivecSum   (const ivec a);


uvec  uvecNew   (uint x, uint y, uint z);
uvec  uvecNewV  (const vec a);
uvec  uvecNewP  (const uint *p);
uvec  uvecZero  ();
uvec  uvecOne   ();
uvec  uvecRng   ();
uvec  uvecNot   (const uvec a);
uvec  uvecIvec  (const ivec a);

uvec  uvecAdd   (const uvec a, const uvec b);
uvec  uvecAddS  (const uvec a, const uint b);
uvec  uvecSub   (const uvec a, const uvec b);
uvec  uvecSubS  (const uvec a, const uint b);
uvec  uvecMul   (const uvec a, const uvec b);
uvec  uvecMulS  (const uvec a, const uint b);
uvec  uvecDiv   (const uvec a, const uvec b);
uvec  uvecDivS  (const uvec a, const uint b);
uvec  uvecShlS  (const uvec a, const uint b);
uvec  uvecShrS  (const uvec a, const uint b);
uvec  uvecAnd   (const uvec a, const uvec b);
uvec  uvecAndS  (const uvec a, const uint b);
uvec  uvecOr    (const uvec a, const uvec b);
uvec  uvecOrS   (const uvec a, const uint b);
uint  uvecOrSum (const uvec a);
uvec  uvecXor   (const uvec a, const uvec b);
uvec  uvecXorS  (const uvec a, const uint b);
