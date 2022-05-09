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

#include "being.h"

#include "../game/being.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../network/messages.h"

#include <stdlib.h>
#include <stdio.h>

#define BEING_LIST_ENTRY_MAX (1<<18)
beingListEntry *beingListEntryList;
beingListEntry *beingListEntryFirstFree;
uint            beingListEntryCount = 0;

u8 beingType(being b){
	return b>>24;
}

u32 beingID(being b){
	return b&0xFFFFFF;
}

being beingNew(u8 type, u32 id){
	return (id&0xFFFFFF) | ((u32)type << 24);
}

being beingCharacter (u32 id){
	return beingNew(bkCharacter, id);
}

being beingEntity (u32 id){
	return beingNew(bkEntity, id);
}

being beingProjectile(u32 id){
	return beingNew(bkProjectile,id);
}

vec beingGetPos(being b){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return vecNOne();}
		return c->pos; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return vecNOne();}
		return e->pos; }
	default:
		return vecNOne();
	}
}

void beingSetPos(being b, const vec pos){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->pos = pos;
		return; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return;}
		e->pos = pos;
		return; }
	default:
		return;
	}
}

void beingAddPos(being b, const vec pos){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->pos = vecAdd(c->pos, pos);
		return; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return;}
		e->pos = vecAdd(e->pos, pos);
		return; }
	default:
		return;
	}
}

vec beingGetVel(being b){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return vecZero();}
		return c->vel; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return vecZero();}
		return e->vel; }
	default:
		return vecZero();
	}
}

void beingSetVel(being b, const vec vel){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->vel = vel;
		return; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return;}
		e->vel = vel;
		return; }
	default:
		return;
	}
}

void beingAddVel(being b, const vec vel){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return;}
		c->vel = vecAdd(c->vel,vel);
		return; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return;}
		e->vel = vecAdd(e->vel, vel);
		return; }
	default:
		return;
	}
}

being beingClosest(const vec pos, float maxDistance){
	character *c = characterClosest(pos,maxDistance);
	if(c != NULL){return characterGetBeing(c);}
	return 0;
}

float beingGetWeight(being b){
	switch(beingType(b)){
	case bkCharacter:
		return 80.f;
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return 1.f;}
		return e->weight; }
	default:
		return 1.f;
	}
}

bool beingGetNoClip(being b){
	switch(beingType(b)){
	case bkCharacter: {
		character *c = characterGetByBeing(b);
		if(c == NULL){return false;}
		return c->flags & CHAR_NOCLIP; }
	case bkEntity: {
		entity *e = entityGetByBeing(b);
		if(e == NULL){return false;}
		return e->flags & ENTITY_NOCLIP; }
	default:
		return false;
	}
}

void beingDamage(being b, i16 hp, u8 cause, float knockbackMult, being culprit, const vec pos){
	if(!isClient){
		msgBeingGotHit(hp,cause,knockbackMult,b,culprit);

		switch(beingType(b)){
		case bkCharacter:
			return msgBeingDamage(beingID(b),hp,cause,knockbackMult,b,culprit,pos);
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
	if(bl != nbl){
		beingListDel( bl,entry);
	}
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

const char *beingGetName(being b){
	switch(beingType(b)){
	case bkCharacter:
		return characterGetName(characterGetByBeing(b));
	default:
		return NULL;
	}
}

static void beingListGetInSphere(beingList *bl, vec pos, float r, being source, void (*callback)(vec pos, being b, being source)){
	if(bl == NULL){return;}
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			const being b = ble->v[i];
			if(b == 0)     {break;}
			if(b == source){continue;}
			const float distance = vecMag(vecSub(pos,beingGetPos(ble->v[i])));
			if(distance < r){
				callback(pos,b,source);
			}
		}
	}
}

void beingGetInSphere(vec pos, float r, being ignore, void (*callback)(vec pos, being b, being source)){
	const int xMax = (int)(pos.x + r) + 1 + 16;
	const int yMax = (int)(pos.y + r) + 1 + 16;
	const int zMax = (int)(pos.z + r) + 1 + 16;
	for(int x = (pos.x - r); x < xMax; x += 16){
	for(int y = (pos.y - r); y < yMax; y += 16){
	for(int z = (pos.z - r); z < zMax; z += 16){
		beingListGetInSphere(beingListGet(x,y,z),pos,r,ignore,callback);
	}
	}
	}
}
