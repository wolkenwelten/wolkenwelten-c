#include "animal.h"

#include "../animals/bunny.h"
#include "../animals/guardian.h"
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

animal  animalList[1<<10];
uint    animalCount = 0;
animal *animalFirstFree = NULL;

#define ANIMALS_PER_UPDATE 16u

animal *animalNew(const vec pos , int type, int gender){
	animal *e = NULL;
	if(animalCount >= ((sizeof(animalList) / sizeof(animal)-1))){return NULL;}
	e = &animalList[animalCount++];
	animalReset(e);

	e->pos       = pos;
	e->rot       = vecZero();
	e->grot      = vecZero();
	e->gvel      = vecZero();

	e->age       = 21;
	e->hunger    = 64;
	e->sleepy    = 64;
	e->pregnancy = -1;

	e->type      = type;
	e->health    = animalGetMaxHealth(e);

	if(rngValM(2) == 0){
		e->flags |= ANIMAL_BELLYSLEEP;
	}
	if(rngValM(2) == 0){
		e->flags |= ANIMAL_AGGRESIVE;
	}
	if(gender < 0){
		if(rngValM(2) == 0){
			e->flags |= ANIMAL_MALE;
		}
	}else if(gender == 1){
		e->flags |= ANIMAL_MALE;
	}

	return e;
}

static void animalDel(uint i){
	if(i >= animalCount) {return;}
	animalList[i] = animalList[--animalCount];
}

void animalUpdateAll(){
	for(int i=animalCount-1;i>=0;i--){
		int dmg = animalUpdate(&animalList[i]);
		animalList[i].health -= dmg;
		if((animalList[i].pos.y  < -256.f) ||
		   (animalList[i].health <= 0) ||
		   (animalList[i].hunger <= 0) ||
		   (animalList[i].sleepy <= 0)) {
			animalRDie(&animalList[i]);
			animalDel(i);
			continue;
		}
	}
}

float animalClosestPlayer(const animal *e, character **cChar){
	*cChar = NULL;
	float ret = 4096.f;
	for(uint i=0;i<clientCount;++i){
		if(clients[i].c == NULL){continue;}
		const float d = vecMag(vecSub(clients[i].c->pos,e->pos));
		if(d < ret){
			ret = d;
			*cChar = clients[i].c;
		}
	}
	return ret;
}

float animalClosestAnimal(const animal *e, animal **cAnim, int typeFilter, uint flagsMask, uint flagsCompare){
	*cAnim = NULL;
	float ret = 4096.f;
	for(uint i=0;i<animalCount;i++){
		if(e == &animalList[i])                                    {continue;}
		if((typeFilter >= 0) && (animalList[i].type != typeFilter)){continue;}
		if((animalList[i].flags & flagsMask) != flagsCompare)      {continue;}
		const float d = vecMag(vecSub(animalList[i].pos,e->pos));
		if(d < ret){
			ret = d;
			*cAnim = &animalList[i];
		}
	}
	return ret;
}

void animalCheckSuffocation(animal *e){
	const u8 cb = worldGetB(e->pos.x,e->pos.y,e->pos.z);
	if(cb != 0){
		if(rngValM(128) == 0){e->health--;}
		const blockCategory cc = blockTypeGetCat(cb);
		if(cc == LEAVES){
			if(rngValM( 512) == 0){worldBoxMine(e->pos.x,e->pos.y,e->pos.z,1,1,1);}
		}else if(cc == DIRT){
			if(rngValM(4096) == 0){worldBoxMine(e->pos.x,e->pos.y,e->pos.z,1,1,1);}
		}
	}
}

void animalRDie(animal *e){
	item drop = itemNew(I_Pear,rngValMM(3,6));
	itemDropNewP(e->pos,&drop);
	msgAnimalDied(-1,e->pos, e->type, e->age);
	addPriorityAnimal(e-animalList);
}


void animalRHit(animal *e){
	if(fabsf(e->vel.y) < 0.001f){
		e->vel.y = 0.03f;
	}
	if(e->state == ANIMAL_S_FLEE || (e->flags & ANIMAL_AGGRESIVE)){
		e->state = ANIMAL_S_FIGHT;
		e->flags |= ANIMAL_AGGRESIVE;
	}else{
		e->state = ANIMAL_S_FLEE;
	}
}

inline static void animalThink(animal *e){
	switch(e->type){
	default:
	case 1:
		animalThinkBunny(e);
		break;
	case 2:
		animalThinkGuardian(e);
		break;
	}
}

void animalThinkAll(){
	for(int i=animalCount-1;i>=0;--i){
		animalThink(&animalList[i]);
	}
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

static void animalSync(u8 c, u16 i){
	packet *rp = &packetBuffer;
	const animal *e = &animalList[i];
	if(!chungusIsSubscribed(e->curChungus,c)){
		return animalSyncInactive(c,i);
	}

	rp->v.u8[ 0] = e->type;
	rp->v.u8[ 1] = e->flags;
	rp->v.u8[ 2] = e->state;
	rp->v.i8[ 3] = e->age;

	rp->v.i8[ 4] = e->health;
	rp->v.i8[ 5] = e->hunger;
	rp->v.i8[ 6] = e->pregnancy;
	rp->v.i8[ 7] = e->sleepy;

	rp->v.u16[4] = i;
	rp->v.u16[5] = animalCount;

	rp->v.f[ 3]  = e->pos.x;
	rp->v.f[ 4]  = e->pos.y;
	rp->v.f[ 5]  = e->pos.z;

	rp->v.f[ 6]  = e->vel.x;
	rp->v.f[ 7]  = e->vel.y;
	rp->v.f[ 8]  = e->vel.z;

	rp->v.f[ 9]  = e->gvel.x;
	rp->v.f[10]  = e->gvel.y;
	rp->v.f[11]  = e->gvel.z;

	rp->v.f[12]  = e->rot.yaw;
	rp->v.f[13]  = e->rot.pitch;

	rp->v.f[14]  = e->grot.yaw;
	rp->v.f[15]  = e->grot.pitch;

	packetQueue(rp,30,16*4,c);
}

uint animalSyncPlayer(u8 c, uint offset){
	const uint max = MIN((offset+ANIMALS_PER_UPDATE),animalCount);
	if(animalCount == 0){
		animalEmptySync(c);
		return offset;
	}
	for(uint i=0;i<clients[c].animalPriorityQueueLen;i++){
		animalSync(c,clients[c].animalPriorityQueue[i]);
	}
	clients[c].animalPriorityQueueLen = 0;

	for(uint i=offset;i<max;i++){
		animalSync(c,i);
	}
	offset += ANIMALS_PER_UPDATE;
	if(offset >= animalCount){offset=0;}
	return offset;
}

void animalDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].curChungus != c){continue;}
		animalDel(i);
	}
}

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

void animalIntro(u8 c){
	for(uint i=0;i<animalCount;i++){
		animalSync(c,i);
	}
}
