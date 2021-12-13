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
#include "lightning.h"


void lightningStrikeRec(const vec a, const vec b, uint stepsLeft, int size, bool branches, void (*fun)(const vec, const vec, int)){
	const vec t = vecAdd(vecAdd(a, vecMulS(vecSub(b,a),0.5f)), vecMulS(vecRng(),stepsLeft * 2));
	if((size > 8) && branches && (stepsLeft < 5)){
		vec branch = vecAdd(vecAdd(vecAdd(a, vecMulS(vecSub(t,a),0.5f)), vecMulS(vecRng(),stepsLeft * 9)), vecNew(0.f, -(vecMag(vecSub(b,a)) / 2), 0.f));
		lightningStrikeRec(a, branch, 4, size-1, branches, fun);
	}
	if(stepsLeft > 0){
		lightningStrikeRec(a,t,stepsLeft-1, size, false, fun);
		lightningStrikeRec(t,b,stepsLeft-1, size, branches, fun);
	}else{
		fun(a,b,size);
	}
}

void lightningStrike(u16 lx, u16 ly, u16 lz, u16 tx, u16 ty, u16 tz, u16 seed, void (*fun)(const vec, const vec, int)){
	const u64 oldSeed = getRNGSeed();
	const u64 newSeed = seed | (tz << 16) | ((u64)ty << 32) | ((u64)tx << 48);

	seedRNG(newSeed);
	lightningStrikeRec(vecNew(lx,ly,lz),vecNew(tx,ty,tz), 5, 10, true, fun);
	seedRNG(oldSeed);
}
