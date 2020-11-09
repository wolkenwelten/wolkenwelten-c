#include "rope.h"

#include "../network/server.h"
#include "../../../common/src/network/messages.h"

int ropeNewID(){
	for(uint i=256;i<512;i++){
		if(ropeList[i].a == 0){return i;}
		if(ropeList[i].b == 0){return i;}
	}
	return -1;
}

void ropeUpdateP(uint c, const packet *p){
	(void)c;

	const uint i    = p->v.u32[0];
	if(i > 512){return;}
	rope *r = &ropeList[i];
	r->a      = p->v.u32[1];
	r->b      = p->v.u32[2];
	r->flags  = p->v.u32[3];
	r->length = p->v.f  [4];
}

void ropeSyncAll(){
	for(uint i=0;i<512;i++){
		if(ropeList[i].a == 0)              {continue;}
		if(ropeList[i].b == 0)              {continue;}
		if(ropeList[i].flags & ROPE_UPDATED){continue;}
		msgRopeUpdate(-1, i, &ropeList[i]);
		ropeList[i].flags |= ROPE_UPDATED;
	}
}

void ropePrioritizeHooked(uint c){
	uint start = ropeGetClient(c);
	for(uint i=start;i<start+4;i++){
		if(ropeList[i].a == 0){continue;}
		if(ropeList[i].b == 0){continue;}
		addPriorityBeing(ropeList[i].a);
		addPriorityBeing(ropeList[i].b);
	}
}
