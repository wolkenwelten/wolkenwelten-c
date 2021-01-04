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
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

void animalDmgPacket(u8 source, const packet *p ){
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
	msgBeingGotHit(hp,cause,1.f,target,culprit);
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
	if(i >= countof(animalList)){return;}
	const animal *e = &animalList[i];
	if(!chungusIsSubscribed(worldTryChungusV(e->pos),c)){
		return animalSyncInactive(c,i);
	}
	return animalSync(c,i);
}

void animalSyncPlayer(u8 c){
	if(animalCount == 0){
		animalEmptySync(c);
		return;
	}

	const u64 mask = 1 << c;
	int count = clients[c].animalUpdateWindowSize;
	for(uint tries = 256;count >= 0;clients[c].animalUpdateOffset++){
		if(--tries == 0){break;}
		const uint i = clients[c].animalUpdateOffset;
		if(i >= animalCount){clients[c].animalUpdateOffset = 0;}
		if(animalList[i].clientPriorization & mask){continue;}
		animalServerSync(c,i);
		count--;
	}

	count = clients[c].animalUpdateWindowSize;
	for(uint tries=2048;count >= 0;clients[c].animalPriorityUpdateOffset++){
		if(--tries == 0){break;}
		const uint i = clients[c].animalPriorityUpdateOffset;
		if(i >= animalCount){clients[c].animalPriorityUpdateOffset = 0;}
		if(!(animalList[i].clientPriorization & mask)){continue;}
		animalServerSync(c,i);
		count--;
	}

	if(getClientLatency(c) < 50){
		clients[c].animalUpdateWindowSize += 1;
	}else{
		clients[c].animalUpdateWindowSize /= 2;
	}
	clients[c].animalUpdateWindowSize = MAX(4,MIN(12,clients[c].animalUpdateWindowSize));
}

void animalDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].type == 0)  {continue;}
		const vec *p = &animalList[i].pos;
		if(((uint)p->x >> 8) != c->x){continue;}
		if(((uint)p->y >> 8) != c->y){continue;}
		if(((uint)p->z >> 8) != c->z){continue;}
		animalDel(i);
	}
}

void animalUpdatePriorities(u8 c){
	const u64 prio = 1 << c;
	const u64 mask = ~(prio);
	uint countPrio = 0;
	if(clients[c].state)     {return;}
	if(clients[c].c == NULL) {return;}
	const vec cpos = clients[c].c->pos;
	for(uint i=0;i<animalCount;i++){
		const float d = vecMag(vecSub(animalList[i].pos,cpos));
		if(d < 78.f){
			animalList[i].clientPriorization |= prio;
			countPrio++;
		}else{
			animalList[i].clientPriorization &= mask;
		}
	}
}
