#include "rope.h"

#include "../game/being.h"
#include "../game/entity.h"
#include "../mods/api_v1.h"

#include <stdio.h>

rope ropeList[128];
uint ropeCount = 0;

rope *ropeNew(being a, being b){
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

	r->a = a;
	r->b = b;
	r->length = 1.f;
	const vec pos = beingGetPos(a);
	for(int i=0;i<16;i++){
		r->nodes[i] = entityNew(pos,vecZero());
		r->nodes[i]->eMesh = meshPear;
	}
	return r;
}

float ropeLength(const rope *r){
	if(r == NULL){return 1.f;}
	return vecMag(vecSub(beingGetPos(r->a),beingGetPos(r->b)));
}

void ropeFree(rope *r){
	if(r == NULL){return;}
	r->a = r->b = 0;
	for(int i=0;i<16;i++){
		if(r->nodes[i] == NULL){continue;}
		entityFree(r->nodes[i]);
		r->nodes[i] = NULL;
	}
}

void ropePullTowards(being a, being b, float goalLen){
	const vec ap = beingGetPos(a);
	const vec bp = beingGetPos(b);
	if(ap.x < 0){return;}
	if(bp.x < 0){return;}
	const float len = vecMag(vecSub(ap,bp));
	const vec d = vecMulS(vecNorm(vecSub(ap,bp)),goalLen - len);

	u32 col = entityCollision(bp);
	vec posAdd = vecZero();
	vec velAdd = vecZero();
	if(((d.x > 0) && !((col & 0x110))) || ((d.x < 0) && !((col & 0x220)))){
		posAdd.x = d.x * -0.5f;
		velAdd.x = d.x * -0.1f;
	}
	if(((d.z > 0) && !((col & 0x440))) || ((d.z < 0) && !((col & 0x880)))){
		posAdd.z = d.z * -0.5f;
		velAdd.z = d.z * -0.1f;
	}
	if(((d.y > 0) && !((col & 0x00F))) || ((d.y < 0) && !((col & 0x0F0)))){
		posAdd.y = d.y * -0.5f;
		velAdd.y = d.y * -0.1f;
	}
	(void)velAdd;
	(void)posAdd;
	beingAddPos(b,posAdd);
	beingAddVel(b,velAdd);
	//pull->shake = vecMag(pull->vel);
}

static void ropeUpdate(rope *r){
	const float segmentLen = r->length / 16.f;
	being a = r->a;
	for(int i=0;i<16;i++){
		being t = entityGetBeing(r->nodes[i]);
		ropePullTowards(a,t,segmentLen);
		a = t;
	}
	ropePullTowards(a,r->b,segmentLen);
}

void  ropeUpdateAll(){
	for(uint i=0;i<ropeCount;i++){
		ropeUpdate(&ropeList[i]);
	}
}
