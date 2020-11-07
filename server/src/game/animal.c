#include "animal.h"

#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../misc/options.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

void animalDmgPacket(u8 source, const packet *p){
	const i16 hp        = p->v.u16[0];
	const u16 cause     = p->v.u16[1];

	const being target  = p->v.u32[1];
	const being culprit = beingCharacter(source);
	if(beingType(target) != BEING_ANIMAL){return;}
	const u16 i   = beingID(target);
	if(i >= animalCount){return;}
	animal *c = &animalList[i];

	c->health -= hp;
	if(c->health <= 0){
		animalRDie(c);

		animalDel(i);
		return;
	}
	animalRHit(c);
	if(cause == 2){
		vec pos = vecNewP(&p->v.f[3]);
		vec dis = vecNorm(vecSub(c->pos,pos));
		c->vel = vecAdd(c->vel,vecMulS(dis,0.03f));
	}
	msgBeingGotHit(hp,cause,target,culprit);
}

static void animalEmptySync(u16 c){
	packet *rp = &packetBuffer;
	memset(rp->v.u8,0,16*4);
	packetQueue(rp,30,16*4,c);
}

static void animalSyncInactive(u8 c, u16 i){
	packet *rp = &packetBuffer;

	rp->v.u8[ 0] = 0;

	rp->v.u16[4] = i;
	rp->v.u16[5] = animalCount;

	packetQueue(rp,30,16*4,c);
}

static void animalServerSync(u8 c, u16 i){
	const animal *e = &animalList[i];
	if(!chungusIsSubscribed(e->curChungus,c)){
		return animalSyncInactive(c,i);
	}
	return animalSync(c,i);
}

uint animalSyncPlayer(u8 c, uint offset){
	const uint max = MIN((offset+clients[c].animalUpdateWindowSize),animalCount);
	if(animalCount == 0){
		animalEmptySync(c);
		return offset;
	}
	for(uint i=0;i<clients[c].animalPriorityQueueLen;i++){
		animalServerSync(c,clients[c].animalPriorityQueue[i]);
	}
	clients[c].animalPriorityQueueLen = 0;

	for(uint i=offset;i<max;i++){
		animalServerSync(c,i);
	}
	offset += clients[c].animalUpdateWindowSize;
	if(offset >= animalCount){offset=0;}
	if(getClientLatency(c) < 100){
		clients[c].animalUpdateWindowSize += 1;
	}else{
		clients[c].animalUpdateWindowSize /= 2;
	}
	clients[c].animalUpdateWindowSize = MAX(1,MIN(8,clients[c].animalUpdateWindowSize));
	return offset;
}

void animalDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(uint i=0;i<animalCount;i++){
		const vec *p = &animalList[i].pos;
		if(((uint)p->x >> 8) != c->x){continue;}
		if(((uint)p->y >> 8) != c->y){continue;}
		if(((uint)p->z >> 8) != c->z){continue;}
		animalDel(i);
	}
}

void animalIntro(u8 c){
	for(uint i=0;i<animalCount;i++){
		animalSync(c,i);
	}
}
