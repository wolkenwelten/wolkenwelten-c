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
#include "entity.h"
#include "nujel.h"
#include "../game/entity.h"
#include "../../nujel/lib/type-system.h"

extern mesh *meshPear;

static entity *requireEntity(lClosure *c, lVal *v){
	const i64 ID = castToInt(v, -1);
	entity *ent = ID >= 0 ? entityGetByID(ID) : NULL;
	if(ent == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't turn that ID into an entity", v, c);
	}
	return ent;
}


static lVal *lEntityNew(lClosure *c, lVal *v){
	(void)c; (void)v;
	entity *ent = entityNew(vecZero(), vecZero(), 1.f);
	ent->flags = ENTITY_NOCLIP | ENTITY_HIDDEN;
	const i64 ID = entityID(ent);
	ent->mesh = meshPear;
	return lValInt(ID);
}

static lVal *lEntityDelete(lClosure *c, lVal *v){
	entityFree(requireEntity(c, lCar(v)));
	return NULL;
}

static lVal *lEntityPosGet(lClosure *c, lVal *v){
	return lValVec(requireEntity(c, lCar(v))->pos);
}

static lVal *lEntityPosSet(lClosure *c, lVal *v){
	entity *ent = requireEntity(c, lCar(v));
	vec pos = requireVec(c, lCadr(v));
	ent->pos = pos;
	return lCar(v);
}

static lVal *lEntityRotGet(lClosure *c, lVal *v){
	return lValVec(requireEntity(c, lCar(v))->rot);
}

static lVal *lEntityRotSet(lClosure *c, lVal *v){
	entity *ent = requireEntity(c, lCar(v));
	vec rot = requireVec(c, lCadr(v));
	ent->rot = rot;
	return lCar(v);
}

static lVal *lEntityVelGet(lClosure *c, lVal *v){
	return lValVec(requireEntity(c, lCar(v))->vel);
}

static lVal *lEntityVelSet(lClosure *c, lVal *v){
	entity *ent = requireEntity(c, lCar(v));
	vec vel = requireVec(c, lCadr(v));
	ent->vel = vel;
	return lCar(v);
}

static lVal *lEntityFlagsGet(lClosure *c, lVal *v){
	return lValInt(requireEntity(c, lCar(v))->flags);
}

static lVal *lEntityFlagsSet(lClosure *c, lVal *v){
	entity *ent = requireEntity(c, lCar(v));
	ent->flags = requireInt(c, lCadr(v));
	return lCar(v);
}

static lVal *lEntityHandlerGet(lClosure *c, lVal *v){
	return requireEntity(c, lCar(v))->handler;
}

static lVal *lEntityHandlerSet(lClosure *c, lVal *v){
	entity *ent = requireEntity(c, lCar(v));
	lVal *handler = lCadr(v);
	if((handler == NULL) || (handler->type != ltLambda)){
		lExceptionThrowValClo("type-error", "Can only use :lambda's as event handlers", handler, c);
	}
	ent->handler = lCadr(v);
	return lCar(v);
}

void lEntityEvent(entity *e, lVal *msg){
	if((e == NULL) || (e->handler == NULL)){return;}
	lApply(clRoot, msg, e->handler, NULL);
}

void lOperatorsEntity(lClosure *c){
	lAddNativeFunc(c,"entity/new*",      "[]",           "Create a new entity and return it's ID", lEntityNew);
	lAddNativeFunc(c,"entity/delete",    "[]",           "Delete the entity E", lEntityDelete);
	lAddNativeFunc(c,"entity/pos",       "[e]",          "Return the position of entity E", lEntityPosGet);
	lAddNativeFunc(c,"entity/pos!",      "[e pos]",      "Set the position of entity E", lEntityPosSet);
	lAddNativeFunc(c,"entity/rotation",  "[e]",          "Return the rotation of entity E", lEntityRotGet);
	lAddNativeFunc(c,"entity/rotation!", "[e rotation]", "Set the rotation of entity E", lEntityRotSet);
	lAddNativeFunc(c,"entity/velocity",  "[e]",          "Return the velocity of entity E", lEntityVelGet);
	lAddNativeFunc(c,"entity/velocity!", "[e velocity]", "Set the velocity of entity E", lEntityVelSet);
	lAddNativeFunc(c,"entity/flags",     "[e]",          "Return the flags for entity E", lEntityFlagsGet);
	lAddNativeFunc(c,"entity/flags!",    "[e flags]",    "Set the flags for entity E", lEntityFlagsSet);
	lAddNativeFunc(c,"entity/handler",   "[e]",          "Return the flags for entity E", lEntityHandlerGet);
	lAddNativeFunc(c,"entity/handler!",  "[e flags]",    "Set the flags for entity E", lEntityHandlerSet);
}
