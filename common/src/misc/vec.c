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


vec vecNewU(const uvec a){
	return (vec){{{a.x,a.y,a.z}}};
}
vec vecNewI(const ivec a){
	return (vec){{{a.x,a.y,a.z}}};
}
int vecInWorld(const vec a){
	if((a.x < 0.f) || (a.y < 0.f) || (a.z < 0.f))            {return 0;}
	if((a.x > 65536.f) || (a.y > 32768.f) || (a.z > 65536.f)){return 0;}
	return 1;
}

ivec ivecNew(int x, int y, int z){
	return (ivec){{{x,y,z}}};
}
ivec ivecNewV(const vec a){
	return (ivec){{{(int)a.x,(int)a.y,(int)a.z}}};
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
ivec ivecXor(const ivec a, const ivec b){
	return (ivec){{{a.x^b.x,a.y^b.y,a.z^b.z}}};
}
ivec ivecXorS(const ivec a, const int b){
	return (ivec){{{a.x^b  ,a.y^b  ,a.z^b  }}};
}
int ivecOrSum(const ivec a){
	return a.x|a.y|a.z;
}
int ivecSum  (const ivec a){
	return a.x+a.y+a.z;
}
bool ivecEq  (const ivec a, const ivec b){
	return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}



uvec uvecNew(uint x, uint y, uint z){
	return (uvec){{{x,y,z}}};
}
uvec uvecNewV(const vec a){
	return (uvec){{{(uint)a.x,(uint)a.y,(uint)a.z}}};
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
bool uvecEq  (const uvec a, const uvec b){
	return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}
