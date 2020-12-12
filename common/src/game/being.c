#include "being.h"

#include "../game/animal.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/grenade.h"
#include "../game/hook.h"
#include "../game/itemDrop.h"
#include "../game/water.h"

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
