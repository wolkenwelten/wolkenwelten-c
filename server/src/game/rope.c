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

#include <stdio.h>
void ropeUpdateP(uint c, const packet *p){
	(void)c;
	const uint i = p->v.u16[0];
	fprintf(stderr,"ropeUpdateP[%u] i:%u\n",c,i);
	if(i > 512){return;}
	rope *r = &ropeList[i];
	r->flags  = p->v.u16[1];
	r->a      = p->v.u32[1];
	r->b      = p->v.u32[2];
	r->length = p->v.f  [3];
}

void ropeSyncAll(){
	for(uint i=0;i<512;i++){
		if(!(ropeList[i].flags & ROPE_DIRTY)){continue;}
		msgRopeUpdate(-1, i, &ropeList[i]);
		ropeList[i].flags &= ~ROPE_DIRTY;
	}
}

void ropeDelBeing(const being t){
	for(uint i = 256;i<512;i++){
		if((ropeList[i].a == t) || (ropeList[i].b == t)){
			ropeList[i].flags = 0;
			ropeList[i].a     = 0;
			ropeList[i].b     = 0;
		}
	}
}
