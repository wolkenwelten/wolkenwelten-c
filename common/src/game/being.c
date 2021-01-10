#include "being.h"

#include "../game/animal.h"
#include "../game/being.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/grenade.h"
#include "../game/hook.h"
#include "../game/itemDrop.h"
#include "../game/water.h"
#include "../network/messages.h"

#include <stdlib.h>
#include <stdio.h>

#define BEING_LIST_ENTRY_MAX (1<<18)
beingListEntry *beingListEntryList;
beingListEntry *beingListEntryFirstFree;
uint            beingListEntryCount = 0;

vec beingGetPos(being b){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return vecNOne();}
		return vecAdd(c->pos,vecNew(0,c->yoff + 0.2f,0)); }
	case BEING_ANIMAL: {
		animal *c = animalGetByBeing(b);
		if(c == NULL){return vecNOne();}
		return c->pos; }
	case BEING_HOOK: {
		hook *c = hookGetByBeing(b);
		if(c == NULL)     {return vecNOne();}
		if(c->ent == NULL){return vecNOne();}
		return c->ent->pos; }
	case BEING_GRENADE: {
		grenade *c = grenadeGetByBeing(b);
		if(c == NULL)     {return vecNOne();}
		if(c->ent == NULL){return vecNOne();}
		return c->ent->pos; }
	case BEING_ITEMDROP: {
		itemDrop *c = itemDropGetByBeing(b);
		if(c == NULL)     {return vecNOne();}
		if(c->ent == NULL){return vecNOne();}
		return c->ent->pos; }
	case BEING_FIRE: {
		fire *c = fireGetByBeing(b);
		if(c == NULL)     {return vecNOne();}
		return vecNew(c->x,c->y,c->z); }
	case BEING_WATER: {
		water *c = waterGetByBeing(b);
		if(c == NULL)     {return vecNOne();}
		return vecNew(c->x,c->y,c->z); }
	default:
		return vecNOne();
	}
}

void beingSetPos(being b, const vec pos){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->pos = pos;
		return; }
	case BEING_ANIMAL: {
		animal *c = animalGetByBeing(b);
		if(c == NULL){return;}
		c->pos = pos;
		return; }
	case BEING_HOOK: {
		hook *c = hookGetByBeing(b);
		if(c == NULL)     {return;}
		if(c->ent == NULL){return;}
		c->ent->pos = pos;
		return; }
	case BEING_GRENADE: {
		grenade *c = grenadeGetByBeing(b);
		if(c == NULL){return;}
		c->ent->pos = pos;
		return; }
	case BEING_ITEMDROP: {
		itemDrop *c = itemDropGetByBeing(b);
		if(c == NULL){return;}
		c->ent->pos = pos;
		return; }
	case BEING_FIRE: {
		fire *c = fireGetByBeing(b);
		if(c == NULL){return;}
		c->x = pos.x;
		c->y = pos.y;
		c->z = pos.z;
		return; }
	case BEING_WATER: {
		water *c = waterGetByBeing(b);
		if(c == NULL){return;}
		c->x = pos.x;
		c->y = pos.y;
		c->z = pos.z;
		return; }
	default:
		return;
	}
}

void beingAddPos(being b, const vec pos){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->pos = vecAdd(c->pos, pos);
		return; }
	case BEING_ANIMAL: {
		animal *c = animalGetByBeing(b);
		if(c == NULL){return;}
		c->pos = vecAdd(c->pos,pos);
		return; }
	case BEING_HOOK: {
		hook *c = hookGetByBeing(b);
		if(c == NULL){return;}
		c->ent->pos = vecAdd(c->ent->pos, pos);
		return; }
	case BEING_GRENADE: {
		grenade *c = grenadeGetByBeing(b);
		if(c == NULL){return;}
		c->ent->pos = vecAdd(c->ent->pos,pos);
		return; }
	case BEING_ITEMDROP: {
		itemDrop *c = itemDropGetByBeing(b);
		if(c == NULL){return;}
		c->ent->pos = vecAdd(c->ent->pos,pos);
		return; }
	case BEING_FIRE: {
		fire *c = fireGetByBeing(b);
		if(c == NULL){return;}
		c->x += pos.x;
		c->y += pos.y;
		c->z += pos.z;
		return; }
	case BEING_WATER: {
		water *c = waterGetByBeing(b);
		if(c == NULL){return;}
		c->x += pos.x;
		c->y += pos.y;
		c->z += pos.z;
		return; }
	default:
		return;
	}
}

vec beingGetVel(being b){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return vecZero();}
		return c->vel; }
	case BEING_ANIMAL: {
		animal *c = animalGetByBeing(b);
		if(c == NULL){return vecZero();}
		return c->vel; }
	case BEING_HOOK: {
		hook *c = hookGetByBeing(b);
		if(c == NULL){return vecZero();}
		return c->ent->vel; }
	case BEING_GRENADE: {
		grenade *c = grenadeGetByBeing(b);
		if(c == NULL){return vecZero();}
		return c->ent->vel; }
	case BEING_ITEMDROP: {
		itemDrop *c = itemDropGetByBeing(b);
		if(c == NULL){return vecZero();}
		return c->ent->vel; }
	case BEING_FIRE:
	case BEING_WATER:
	default:
		return vecZero();
	}
}

void beingSetVel(being b, const vec vel){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->vel = vel;
		return; }
	case BEING_ANIMAL: {
		animal *c = animalGetByBeing(b);
		if(c == NULL){return;}
		c->vel = vel;
		return; }
	case BEING_HOOK: {
		hook *c = hookGetByBeing(b);
		if(c == NULL){return;}
		c->ent->vel = vel;
		return; }
	case BEING_GRENADE: {
		grenade *c = grenadeGetByBeing(b);
		if(c == NULL){return;}
		c->ent->vel = vel;
		return; }
	case BEING_ITEMDROP: {
		itemDrop *c = itemDropGetByBeing(b);
		if(c == NULL){return;}
		c->ent->vel = vel;
		return; }
	case BEING_FIRE:
	case BEING_WATER:
	default:
		return;
	}
}

void beingAddVel(being b, const vec vel){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->vel = vecAdd(c->vel,vel);
		return; }
	case BEING_ANIMAL: {
		animal *c = animalGetByBeing(b);
		if(c == NULL){return;}
		c->vel = vecAdd(c->vel,vel);
		return; }
	case BEING_HOOK: {
		hook *c = hookGetByBeing(b);
		if(c == NULL){return;}
		c->ent->vel = vecAdd(c->ent->vel,vel);
		return; }
	case BEING_GRENADE: {
		grenade *c = grenadeGetByBeing(b);
		if(c == NULL){return;}
		c->ent->vel = vecAdd(c->ent->vel,vel);
		return; }
	case BEING_ITEMDROP: {
		itemDrop *c = itemDropGetByBeing(b);
		if(c == NULL){return;}
		c->ent->vel = vecAdd(c->ent->vel,vel);
		return; }
	case BEING_FIRE:
	case BEING_WATER:
	default:
		return;
	}
}

being beingClosest(const vec pos, float maxDistance){
	character *c = characterClosest(pos,maxDistance);
	if(c != NULL){return characterGetBeing(c);}
	animal *a = animalClosest(pos,maxDistance);
	if(a != NULL){return animalGetBeing(a);}
	return 0;
}

float beingGetWeight(being b){
	switch(beingType(b)){
	case BEING_CHARACTER:
		return 80.f;
	case BEING_ANIMAL:
		return 10.f;
	case BEING_HOOK:
		return 1.f;
	case BEING_GRENADE:
		return 1.f;
	default:
		return 1.f;
	}
}

void beingDamage(being b, i16 hp, u8 cause, float knockbackMult, being culprit, const vec pos){
	if(!isClient){
		msgBeingGotHit(hp,cause,knockbackMult,b,culprit);

		switch(beingType(b)){
		case BEING_CHARACTER:
			return msgBeingDamage(beingID(b),hp,cause,knockbackMult,b,culprit,pos);
		case BEING_ANIMAL:
			return animalDoDamage(animalGetByBeing(b),hp,cause,knockbackMult,culprit,pos);
		}
	}else{
		msgBeingDamage(beingID(b),hp,cause,knockbackMult,b,culprit,pos);
	}
}

bool beingAlive(being b){
	if(b == 0){return false;}
	switch(beingType(b)){
	default:
		return true;
	case BEING_ANIMAL: {
		animal *a = animalGetByBeing(b);
		if(a == NULL)     {return false;}
		if(a->type == 0)  {return false;}
		if(a->health <= 0){return false;}
		return true;
		}
	}
}

void beingListAdd(beingList *bl, being entry){
	beingListEntry *lastEntry = NULL;
	if(bl == NULL){return;}
	if(entry == 0){return;}
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		lastEntry = ble;
		for(uint i=0;i<countof(ble->v);i++){
			if(entry == ble->v[i]){return;}
			if(ble->v[i] == 0){
				ble->v[i] = entry;
				return;
			}
		}
	}
	if(lastEntry == NULL){
		bl->first = beingListEntryNew();
		if(bl->first == NULL){
			fprintf(stderr,"bl->first == NULL\n");
			return;
		}
		bl->first->v[0] = entry;
	}else{
		lastEntry->next = beingListEntryNew();
		if(lastEntry->next == NULL){
			fprintf(stderr,"lastEntry->next == NULL\n");
			return;
		}
		lastEntry->next->v[0] = entry;
	}
}

void beingListDel(beingList *bl, being entry){
	beingListEntry *lastLastEntry = NULL;
	beingListEntry *lastEntry = NULL;
	being *curEntry = NULL;
	if(bl == NULL){return;}
	if(entry == 0){return;}
	beingListDel(bl->parent,entry);
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		lastLastEntry = lastEntry;
		lastEntry = ble;
		if(curEntry != NULL){continue;}
		for(uint i=0;i<countof(ble->v);i++){
			if(entry == ble->v[i]){
				curEntry = &ble->v[i];
				break;
			}
			if(ble->v[i] == 0){return;}
		}
	}
	if(lastEntry == NULL){return;}
	if(lastEntry->v[0] == 0){
		beingListEntryFree(lastEntry);
		if(lastLastEntry == NULL){
			bl->first = NULL;
		}else{
			lastLastEntry->next = NULL;
		}
		return;
	}
	for(uint i=1;i<countof(lastEntry->v);i++){
		if(lastEntry->v[i] == 0){
			*curEntry = lastEntry->v[i-1];
			lastEntry->v[i-1] = 0;
		}
	}
}

void beingListInit(beingList *bl, beingList *parent){
	bl->first  = NULL;
	bl->count  = 0;
	bl->parent = parent;
}

void beingListEntryInit(){
	beingListEntryList = malloc(sizeof(beingListEntry) * BEING_LIST_ENTRY_MAX);
	if(beingListEntryList == NULL){
		fprintf(stderr,"Couldn't allocate beingListEntryList, exiting!\n");
		exit(6);
	}
	for(int i=0;i<BEING_LIST_ENTRY_MAX-1;i++){
		beingListEntryList[i].next = &beingListEntryList[i+1];
	}
	beingListEntryFirstFree = &beingListEntryList[0];
}

beingListEntry *beingListEntryNew(){
	beingListEntry *ret = beingListEntryFirstFree;
	if(ret == NULL){return NULL;}
	beingListEntryFirstFree = ret->next;
	ret->next = NULL;
	return ret;
}

void beingListEntryFree(beingListEntry *ble){
	ble->next = beingListEntryFirstFree;
	beingListEntryFirstFree = ble;
}

beingList *beingListUpdate(beingList *bl, being entry){
	if(entry == 0){return NULL;}
	vec pos = beingGetPos(entry);
	beingList *nbl = beingListGet(pos.x,pos.y,pos.z);
	if(bl == nbl){return bl;}
	beingListDel( bl,entry);
	beingListAdd(nbl,entry);
	return nbl;
}

void beingListPrint(beingList *bl){
	if(bl == NULL){return;}
	printf("[%p]=",bl);
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			if(ble->v[i] == 0){break;}
			printf("%i ",beingID(ble->v[i]));
		}
		printf("\n");
	}
	printf("\n");
}

being beingListGetClosest(const beingList *bl, const being source, uint type, float *d){
	float maxD = 99999.f;
	being maxB = 0;
	if(bl == NULL){return 0;}
	const vec pos = beingGetPos(source);
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			const being b = ble->v[i];
			if(b == 0)     {break;}
			if(b == source){continue;}
			if(beingType(b) != type){continue;}
			float cd = vecMag(vecSub(pos,beingGetPos(ble->v[i])));
			if(cd > maxD){continue;}
			maxD = cd;
			maxB = b;
		}
	}
	if(maxB == 0){
		return beingListGetClosest(bl->parent,source,type,d);
	}
	if(d != NULL){*d = maxD;}
	return maxB;
}
