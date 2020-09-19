#pragma once
#include "../gfx_structs.h"

vec   vecNew     (float x, float y, float z);
vec   vecNewP    (const float *p);
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
float vecMag     (const vec a);
vec   vecFloor   (const vec a);
vec   vecNorm    (const vec a);
vec   vecSqrt    (const vec a);
vec   vecCross   (const vec a, const vec   b);
vec   vecRotate  (const vec a, const vec   b, const float rad);
vec   vecVecToDeg(const vec a);
vec   vecDegToVec(const vec a);
