#include "rope.h"

#include "../game/being.h"
#include "../game/entity.h"
#include "../mods/api_v1.h"

#include <stdio.h>
#include <string.h>

rope ropeList[128];
uint ropeCount = 0;

rope *ropeNew(being a, being b, u32 flags){
	rope *r = NULL;
	for(uint i=0;i<ropeCount;i++){
		if(ropeList[i].a == 0){
			r = &ropeList[i];
			break;
		}
	}
	if(r == NULL){
		if(ropeCount >= sizeof(ropeList)/sizeof(rope)){
			fprintf(stderr,"ropeList is full\n");
			return NULL;
		}
		r = &ropeList[ropeCount++];
	}

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

void ropeFree(rope *r){
	if(r == NULL){return;}
	memset(r,0,sizeof(rope));
}

static void ropePullTowards(being a, being b, float goalLen, float mul){
	const vec ap = beingGetPos(a);
	const vec bp = beingGetPos(b);
	//fprintf(stderr,"AP[%f:%f:%f]{%x} BP[%f:%f:%f]{%x} GL:%f \n",ap.x,ap.y,ap.z,a,bp.x,bp.y,bp.z,b,goalLen);
	if(ap.x < 0){return;}
	if(bp.x < 0){return;}
	const float len = vecMag(vecSub(ap,bp));
	const vec d = vecMulS(vecNorm(vecSub(ap,bp)),goalLen - len);

	u32 col = entityCollision(bp);
	vec posAdd = vecZero();
	vec velAdd = vecZero();
	if(((d.x > 0) && !((col & 0x110))) || ((d.x < 0) && !((col & 0x220)))){
		posAdd.x = d.x * -0.5f * mul;
		velAdd.x = d.x * -0.1f * mul;
	}
	if(((d.z > 0) && !((col & 0x440))) || ((d.z < 0) && !((col & 0x880)))){
		posAdd.z = d.z * -0.5f * mul;
		velAdd.z = d.z * -0.1f * mul;
	}
	if(((d.y > 0) && !((col & 0x00F))) || ((d.y < 0) && !((col & 0x0F0)))){
		posAdd.y = d.y * -0.5f * mul;
		velAdd.y = d.y * -0.1f * mul;
	}
	(void)velAdd;
	(void)posAdd;
	//fprintf(stderr,"AP[%f:%f:%f] BP[%f:%f:%f] L:%f GL:%f PA:[%f:%f:%f] VA:[%f:%f:%f]\n",ap.x,ap.y,ap.z,bp.x,bp.y,bp.z,len,goalLen,posAdd.x,posAdd.y,posAdd.z,velAdd.x,velAdd.y,velAdd.z);
	beingAddPos(b,posAdd);
	beingAddVel(b,velAdd);
	//pull->shake = vecMag(pull->vel);
}

static void ropeUpdate(rope *r){
	if(r == NULL)      { return; }
	if(r->a == 0)      { return; }
	if(r->b == 0)      { return; }
	if(r->length < 0.f){ return; }

	float aw = beingGetWeight(r->a);
	float bw = beingGetWeight(r->b);
	float w  = aw+bw;

	float bm = MIN(1.f,aw / w);
	float am = MIN(1.f,bw / w);
	if(beingType(r->a) == BEING_HOOK){
		am = 0.f;
		bm = 1.f;
	}else if(beingType(r->b) == BEING_HOOK){
		am = 1.f;
		bm = 0.f;
	}

	ropePullTowards(r->a,r->b,r->length,bm);
	ropePullTowards(r->b,r->a,r->length,am);
}

void  ropeUpdateAll(){
	for(uint i=0;i<ropeCount;i++){
		ropeUpdate(&ropeList[i]);
	}
}
