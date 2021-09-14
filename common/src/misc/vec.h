#pragma once
#include "../gfx_structs.h"

vec   vecNewI     (const ivec a);
int   vecInWorld  (const vec a);

ivec ivecNew      (int x, int y, int z);
ivec ivecNewV     (const vec  a);
ivec ivecNewP     (const int *p);
ivec ivecZero     ();
ivec ivecOne      ();
ivec ivecNOne     ();
ivec ivecRng      ();
ivec ivecInvert   (const ivec a);
ivec ivecAdd      (const ivec a, const ivec b);
ivec ivecAddS     (const ivec a, const int  b);
ivec ivecSub      (const ivec a, const ivec b);
ivec ivecSubS     (const ivec a, const int  b);
ivec ivecMul      (const ivec a, const ivec b);
ivec ivecMulS     (const ivec a, const int  b);
ivec ivecDiv      (const ivec a, const ivec b);
ivec ivecDivS     (const ivec a, const int  b);
ivec ivecShlS     (const ivec a, const int  b);
ivec ivecShrS     (const ivec a, const int  b);
ivec ivecAnd      (const ivec a, const ivec b);
ivec ivecAndS     (const ivec a, const int  b);
ivec ivecOr       (const ivec a, const ivec b);
ivec ivecOrS      (const ivec a, const int  b);
ivec ivecXor      (const ivec a, const ivec b);
ivec ivecXorS     (const ivec a, const int  b);
int  ivecOrSum    (const ivec a);
int  ivecSum      (const ivec a);
bool ivecEq       (const ivec a, const ivec b);
