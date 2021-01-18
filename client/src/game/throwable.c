#include "throwable.h"

#include "character.h"
#include "entity.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

void throwableNew(const vec pos, const vec rot, float speed, const item itm, being thrower, u16 flags){
	packet    *p = &packetBuffer;
	const uint counter = 0;
	const vec vel  = vecMulS(vecDegToVec(rot),speed);

	p->v.u16[ 0] = 0;
	p->v.u16[ 1] = 0;

	p->v.u16[ 2] = counter;
	p->v.u16[ 3] = flags;
	p->v.u16[ 4] = itm.ID;
	p->v.u16[ 5] = itm.amount;

	p->v.u32[ 3] = thrower;

	p->v.f  [ 4] = pos.x;
	p->v.f  [ 5] = pos.y;
	p->v.f  [ 6] = pos.z;

	p->v.f  [ 7] = vel.x;
	p->v.f  [ 8] = vel.y;
	p->v.f  [ 9] = vel.z;

	p->v.f  [10] = -rot.yaw;
	p->v.f  [11] = rot.pitch;

	packetQueue(p,45,12*4,0);
}

static void throwableDel(uint i){
	packet    *p = &packetBuffer;

	p->v.u16[ 0] = i;
	p->v.u16[ 1] = 1;

	packetQueue(p,45,12*4,0);

	throwableList[i].nextFree = 0;
	entityFree(throwableList[i].ent);
	throwableList[i].ent = NULL;
}

void throwableCheckPickup(){
	static uint calls = 0;
	PROFILE_START();

	for(uint i=(++calls&0xF);i<throwableCount;i+=0x10){
		throwable *t = &throwableList[i];
		if(t->nextFree   >= 0){continue;}
		if(t->itm.amount <= 0){continue;}
		if(t->flags & THROWABLE_COLLECTABLE){
			vec dist = vecSub(player->pos,t->ent->pos);
			float dd = vecDot(dist,dist);
			if(dd < 2.f * 2.f){
				characterPickupItem(player,t->itm.ID,t->itm.amount);
				throwableDel(i);
			}
		}
	}

	PROFILE_STOP();
}
