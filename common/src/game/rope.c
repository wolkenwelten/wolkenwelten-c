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
	float len = 0;
	const vec ap = beingGetPos(r->a);
	const vec bp = beingGetPos(r->b);
	len += vecMag(vecSub(ap,r->nodes[0]->pos));
	for(int i=0;i<15;i++){
		len += vecMag(vecSub(r->nodes[i]->pos,r->nodes[i+1]->pos));
	}
	len += vecMag(vecSub(bp,r->nodes[15]->pos));
	return len;
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
	//fprintf(stderr,"AP[%f:%f:%f]{%x} BP[%f:%f:%f]{%x} GL:%f \n",ap.x,ap.y,ap.z,a,bp.x,bp.y,bp.z,b,goalLen);
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
	//fprintf(stderr,"AP[%f:%f:%f] BP[%f:%f:%f] L:%f GL:%f PA:[%f:%f:%f] VA:[%f:%f:%f]\n",ap.x,ap.y,ap.z,bp.x,bp.y,bp.z,len,goalLen,posAdd.x,posAdd.y,posAdd.z,velAdd.x,velAdd.y,velAdd.z);
	beingAddPos(b,posAdd);
	beingAddVel(b,velAdd);
	//pull->shake = vecMag(pull->vel);
}

static void ropeUpdate(rope *r){
	float segmentLen;
	if(r == NULL){return;}
	if(r->a == 0){return;}
	if(r->b == 0){return;}
	if(r->length > 0.f){
		segmentLen = r->length / 16.f;
	}else{
		segmentLen = ropeLength(r) / 16.f;
	}
	being a = r->b;
	for(int i=0;i<16;i++){
		being t = entityGetBeing(r->nodes[i]);
		ropePullTowards(a,t,segmentLen);
		a = t;
	}
	//ropePullTowards(a,r->b,segmentLen);
}

void  ropeUpdateAll(){
	for(uint i=0;i<ropeCount;i++){
		ropeUpdate(&ropeList[i]);
	}
}
