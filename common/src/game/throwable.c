#include "throwable.h"

#include "entity.h"
#include "../network/messages.h"
#include "../mods/mods.h"
#include "../misc/profiling.h"

throwable throwableList[2048];
uint      throwableCount = 0;
i16       throwableFirstFree = -1;

throwable *throwableGetByBeing(being b){
	if(beingType(b) != BEING_THROWABLE){return NULL;}
	uint i = beingID(b);
	if(i >= throwableCount){return NULL;}
	return &throwableList[i];
}

being throwableGetBeing(const throwable *t){
	if(t == NULL){return 0;}
	return beingThrowable(t - throwableList);
}

throwable *throwableAlloc(){
	if(throwableFirstFree >= 0){
		throwable *t = &throwableList[throwableFirstFree];
		throwableFirstFree = t->nextFree;
		t->nextFree = -1;
		return t;
	}
	throwable *t = &throwableList[throwableCount++];
	t->nextFree = -1;
	return t;
}

void throwableFree(throwable *t){
	if(t->ent != NULL){entityFree(t->ent);}
	t->ent = NULL;
	t->nextFree = throwableFirstFree;
	throwableFirstFree = t - throwableList;
}

void throwableSendUpdate(uint c, uint i){
	if(i >= throwableCount){return;}
	throwable *a = &throwableList[i];
	packet    *p = &packetBuffer;
	if(a->ent == NULL){return;}

	p->v.u16[ 0] = i;
	p->v.u16[ 1] = throwableCount;

	p->v.u16[ 2] = a->counter;
	p->v.u16[ 3] = a->flags;
	p->v.u16[ 4] = a->itm.ID;
	p->v.u16[ 5] = a->itm.amount;

	if(a->itm.amount == 0){
		throwableFree(a);
		return;
	}

	p->v.u32[ 3] = a->thrower;

	p->v.f  [ 4] = a->ent->pos.x;
	p->v.f  [ 5] = a->ent->pos.y;
	p->v.f  [ 6] = a->ent->pos.z;

	p->v.f  [ 7] = a->ent->vel.x;
	p->v.f  [ 8] = a->ent->vel.y;
	p->v.f  [ 9] = a->ent->vel.z;

	p->v.f  [10] = a->ent->rot.yaw;
	p->v.f  [11] = a->ent->rot.pitch;

	packetQueue(p,45,12*4,c);
}

void throwableRecvUpdate(const packet *p){
	const uint i    = p->v.u16[0];
	const uint nlen = p->v.u16[1];

	throwable *a = NULL;
	if(isClient){
		a = &throwableList[i];
		for(uint ii=nlen;ii<throwableCount;ii++){
			throwableFree(&throwableList[ii]);
		}
		throwableCount = nlen;
		a->nextFree = -1;
	}else{
		a = throwableAlloc();
		if(nlen > 0){
			throwableFree(a = &throwableList[i]);
			throwableList[i].itm.amount = 0;
			throwableSendUpdate(-1,i);
			return;
		}
		a->nextFree = -1;
	}

	a->counter    = p->v.u16[ 2];
	a->flags      = p->v.u16[ 3];
	a->itm.ID     = p->v.u16[ 4];
	a->itm.amount = p->v.u16[ 5];

	a->thrower    = p->v.u32[ 3];

	if(a->ent == NULL){
		a->ent = entityNew(vecNewP(&p->v.f[ 4]),vecZero());
		if(isClient){
			a->ent->eMesh = getMeshDispatch(&a->itm);
		}
	}else{
		a->ent->pos = vecNewP(&p->v.f[ 4]);
	}
	a->ent->vel       = vecNewP(&p->v.f[ 7]);
	a->ent->rot.yaw   = p->v.f[10];
	a->ent->rot.pitch = p->v.f[11];
	a->ent->rot.roll  = 0;

	if(!isClient){
		throwableSendUpdate(-1,a - throwableList);
	}
}

static void throwableUpdate(throwable *t){
	if(t->ent == NULL){return;}
	entityUpdate(t->ent);
	if((t->flags & THROWABLE_PIERCE_BLOCK) && (t->ent->flags & ENTITY_COLLIDE)){
		t->ent->flags |= ENTITY_NOCLIP;
		t->flags &= ~THROWABLE_PITCH_SPIN;
		t->flags |= THROWABLE_COLLECTABLE;
		t->ent->vel = vecZero();
	}
	if(t->flags & THROWABLE_PITCH_SPIN){
		t->ent->rot.pitch -= 2.f;
	}
}

void throwableUpdateAll(){
	PROFILE_START();

	for(uint i=0;i<throwableCount;i++){
		throwable *t = &throwableList[i];
		if(t->nextFree >= 0){continue;}
		throwableUpdate(t);
	}

	PROFILE_STOP();
}
