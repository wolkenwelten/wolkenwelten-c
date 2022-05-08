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

#include "rope.h"

#include "../game/being.h"
#include "../game/entity.h"
#include "../network/messages.h"

#include <string.h>

rope ropeList[512];

rope *ropeGetByID(uint i){
	if(i == 0){return NULL;}
	if(i-1 >= countof(ropeList)){return NULL;}
	rope *r = &ropeList[i-1];
	if((r->a == 0) || (r->b == 0)){return NULL;}
	return r;
}

uint ropeGetID(const rope *r){
	if(r == NULL){return 0;}
	return (r - ropeList)+1;
}

rope *ropeNew(being a, being b, u32 flags){
	const int newID = ropeNewID();
	if(newID < 0){return NULL;}
	rope *r = &ropeList[newID];
	r->a      = a;
	r->b      = b;
	r->length = 1.f;
	r->flags  = flags;
	return r;
}

float ropeGetLength(const rope *r){
	if(r == NULL){return 1.f;}
	const vec ap = beingGetPos(r->a);
	const vec bp = beingGetPos(r->b);
	return vecMag(vecSub(ap,bp));
}

void ropeSetLength(rope *r, float l){
	if(r == NULL){return;}
	r->length = l;
}

void ropeFree(rope *r){
	if(r == NULL){return;}
	memset(r,0,sizeof(rope));
}

static void ropePullTowards(being a, being b, float goalLen, float mul){
	const vec ap = beingGetPos(a);
	const vec bp = beingGetPos(b);
	if((ap.x < 0) || (bp.x < 0)){return;}
	const float len = vecMag(vecSub(ap,bp));
	if(goalLen > len){return;}
	const vec d = vecMulS(vecNorm(vecSub(ap,bp)),goalLen - len);

	u32 col    = entityCollision(bp);
	vec posAdd = vecZero();
	vec velAdd = vecZero();
	if(((d.x > 0) && !((col & 0x110))) || ((d.x < 0) && !((col & 0x220)))){
		posAdd.x = d.x * -0.6f * mul;
		velAdd.x = MINMAX(d.x * -0.1f * mul,-0.01,0.01);
	}
	if(((d.z > 0) && !((col & 0x440))) || ((d.z < 0) && !((col & 0x880)))){
		posAdd.z = d.z * -0.6f * mul;
		velAdd.z = MINMAX(d.z * -0.1f * mul,-0.01,0.01);
	}
	if(((d.y > 0) && !((col & 0x00F))) || ((d.y < 0) && !((col & 0x0F0)))){
		posAdd.y = d.y * -0.6f * mul;
		velAdd.y = MINMAX(d.y * -0.1f * mul,-0.01,0.01);
	}
	beingAddPos(b,posAdd);
	beingAddVel(b,velAdd);
}

static void ropeUpdate(rope *r){
	if(r->a == 0)      { return; }
	if(r->b == 0)      { return; }
	if(r->length < 0.f){ return; }

	float aw = beingGetWeight(r->a);
	float bw = beingGetWeight(r->b);
	float w  = aw+bw;

	float bm = MAX(0.01f,MIN(1.f,aw / w));
	float am = MAX(0.01f,MIN(1.f,bw / w));

	ropePullTowards(r->b,r->a,r->length,am);
	ropePullTowards(r->a,r->b,r->length,bm);
}

void ropeUpdateAll(){
	for(uint i=0;i<512;i++){
		ropeUpdate(&ropeList[i]);
	}
}

int ropeGetClient(uint i){
	if(i > 512){return -1;}
	if(i > 256){return 64;}
	return i >> 2;
}
