/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "throwable.h"

#include "entity.h"
#include "../game/animal.h"
#include "../game/item.h"
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
		t->ent = NULL;
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

void throwableDel(uint i){
	throwableFree(&throwableList[i]);
}

void throwableSendUpdate(int c, uint i){
	if(i >= throwableCount){return;}
	throwable *a = &throwableList[i];
	packet    *p = &packetBuffer;
	if(a->ent == NULL){return;}

	p->v.u16[ 0] = i;
	p->v.u16[ 1] = throwableCount;

	p->v.u16[ 2] = a->counter;
	p->v.u8 [ 6] = a->damage;
	p->v.u8 [ 7] = a->flags;
	p->v.u16[ 4] = a->itm.ID;
	p->v.u16[ 5] = a->itm.amount;

	if(a->itm.amount == 0){
		throwableFree(a);
		packetQueue(p,msgtThrowableRecvUpdates,12*4,c);
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

	packetQueue(p,msgtThrowableRecvUpdates,12*4,c);
}

void throwableEmptyUpdate(int c){
	packet *p = &packetBuffer;

	p->v.u32[ 0] = 0;
	p->v.u32[ 2] = 0;

	packetQueue(p,msgtThrowableRecvUpdates,12*4,c);
}

void throwableRecvUpdate(const packet *p){
	const uint i    = p->v.u16[0];
	const uint nlen = p->v.u16[1];
	const uint itmAmount = p->v.u16[ 5];

	throwable *a = NULL;
	if(isClient){
		if(itmAmount == 0){
			throwableFree(&throwableList[i]);
			throwableList[i].itm.amount = 0;
			return;
		}
		a = &throwableList[i];
		for(uint ii=nlen;ii<throwableCount;ii++){
			throwableFree(&throwableList[ii]);
		}
		throwableCount = nlen;
		a->nextFree = -1;
	}else{
		if(nlen > 0){
			throwableFree(&throwableList[i]);
			throwableList[i].itm.amount = 0;
			throwableSendUpdate(-1,i);
			return;
		}else{
			a = throwableAlloc();
			a->nextFree = -1;
		}
	}

	a->counter    = p->v.u16[ 2];
	a->damage     = p->v.u8 [ 6];
	a->flags      = p->v.u8 [ 7];
	a->itm.ID     = p->v.u16[ 4];
	a->itm.amount = itmAmount;

	a->thrower    = p->v.u32[ 3];

	if(a->ent == NULL){
		a->ent = entityNew(vecNewP(&p->v.f[ 4]),vecZero());
		if(isClient){
			a->ent->eMesh = itemGetMesh(&a->itm);
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
	if((t->flags & THROWABLE_PIERCE) && (t->ent->flags & ENTITY_COLLIDE)){
		const u8 b = entityGetBlockCollision(t->ent);
		const blockCategory cat = blockTypeGetCat(b);
		if(cat != STONE){
			t->ent->flags |= ENTITY_NOCLIP;
			t->ent->vel = vecZero();
		}else{
			t->ent->pos = vecAdd(t->ent->pos,t->ent->vel);
			t->ent->vel = vecMulS(t->ent->vel,0.2f);
		}
		t->flags &= ~(THROWABLE_PITCH_SPIN | THROWABLE_TIP_HEAVY);
		t->flags |= THROWABLE_COLLECTABLE;
	}
	/*if(characterHitCheck(p->pos, mdd, 1, 3, iteration, p->source))*/
	if((t->flags & THROWABLE_PIERCE) && (t->ent != NULL)){
		if(animalHitCheck(t->ent->pos, 1.f, t->damage, 3, -1, 0)){
			t->ent->flags &= ~THROWABLE_PIERCE;
			t->ent->vel = vecMulS(t->ent->vel,-0.1f);
			t->ent->vel.y = 0.01f;
		}
	}
	if(t->flags & THROWABLE_PITCH_SPIN){
		t->ent->rot.pitch -= 2.f;
	}
	if(t->flags & THROWABLE_TIP_HEAVY){
		if(t->ent->rot.pitch > -90){
			t->ent->rot.pitch -= 0.5f;
		}else{
			t->ent->rot.pitch = -90.f;
		}
	}
	if(!isClient && !(t->ent->flags & ENTITY_NOCLIP) && (t->ent->flags & ENTITY_COLLIDE) && (vecAbsSum(t->ent->vel) < 0.01f)){
		itemDropNewP(t->ent->pos,&t->itm);
		t->itm.amount = 0;
		t->itm.ID = 0;
		throwableSendUpdate(-1,t - throwableList);
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

bool throwableTry(item *cItem,character *cChar, float strength, int damage, uint flags){
	if(characterIsThrowAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		characterAddRecoil(cChar,1.f);
		if(itemGetStackSize(cItem) == 1){
			throwableNew(cChar->pos, cChar->rot, strength, *cItem, characterGetBeing(cChar), damage, flags);
		}else{
			item tmp;
			tmp = *cItem;
			tmp.amount = 1;
			throwableNew(cChar->pos, cChar->rot, strength, tmp, characterGetBeing(cChar), damage, flags);
		}
		itemDecStack(cItem,1);
		if(itemIsEmpty(cItem)){
			characterStopAim(cChar);
		}else{
			characterStartAnimation(cChar,0,240);
		}
		return true;
	}
	return false;
}

bool throwableTryAim(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleThrowAim(cChar,2.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

void throwableDelChungus(chungus *c){
	if(c == NULL){return;}
	const ivec cp = chungusGetPos(c);
	for(uint i=0;i<throwableCount;i++){
		if(throwableList[i].ent == NULL){continue;}
		const vec *p = &throwableList[i].ent->pos;
		if(((int)p->x >> 8) != cp.x){continue;}
		if(((int)p->y >> 8) != cp.y){continue;}
		if(((int)p->z >> 8) != cp.z){continue;}
		throwableDel(i);
	}
}
