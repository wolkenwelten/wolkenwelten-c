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
#include "../game/being.h"
#include "../game/entity.h"
#include "../game/rope.h"
#include "../../nujel/lib/type-system.h"


static rope *requireRope(lClosure *c, lVal *v){
	const i64 ID = requireInt(c, v);
	rope *r = ID > 0 ? ropeGetByID(ID) : NULL;
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't turn that ID into an rope", v, c);
	}
	return r;
}

static being requireBeing(lClosure *c, lVal *v){
	const being ID = requireInt(c, v);
	return ID;
}

static lVal *lRopeNew(lClosure *c, lVal *v){
	being a = requireBeing(c,  lCar(v));
	being b = requireBeing(c, lCadr(v));
	rope *r = ropeNew(a, b, ROPE_DIRTY);
	r->length = 32;
	const int ID = ropeGetID(r);
	return lValInt(ID);
}

static lVal *lRopeLengthGet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	return lValFloat(r->length);
}

static lVal *lRopeFlagsGet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	return lValInt(r->flags);
}

static lVal *lRopeLengthSet(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	rope *r = requireRope(c, car);
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	r->length = requireFloat(c, lCadr(v));
	return car;
}

static lVal *lRopeFlagsSet(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	rope *r = requireRope(c, car);
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	r->flags = requireInt(c, lCadr(v));
	return car;
}

static lVal *lRopeDistanceGet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	return lValFloat(vecMag(vecSub(beingGetPos(r->a), beingGetPos(r->b))));
}

static lVal *lRopeValidP(lClosure *c, lVal *v){
	const i64 ID = requireInt(c, lCar(v));
	rope *r = ID > 0 ? ropeGetByID(ID) : NULL;
	if(r == NULL){
		return lValBool(false);
	}
	return lValBool(r->a && r->b);
}

static lVal *lRopeGetA(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	if(beingType(r->a) != bkEntity){
		lExceptionThrowValClo("invalid-reference", "Can only return entities, no other beings for now", lCar(v), c);
	}
	const i64 ID = entityID(entityGetByBeing(r->a));
	return lValInt(ID);
}

static lVal *lRopeGetB(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	if(beingType(r->b) != bkEntity){
		lExceptionThrowValClo("invalid-reference", "Can only return entities, no other beings for now", lCar(v), c);
	}
	const i64 ID = entityID(entityGetByBeing(r->b));
	return lValInt(ID);
}

void lOperatorsRope(lClosure *c){
	lAddNativeFunc(c, "rope/new*",     "[a b]",         "Create a new rope connecting beings A and B", lRopeNew);
	lAddNativeFunc(c, "rope/valid?",   "[rope]",        "Returns #t if ROPE is valid", lRopeValidP);
	lAddNativeFunc(c, "rope/distance", "[rope]",        "Returns the distance between the two beings connected via ROPE", lRopeDistanceGet);
	lAddNativeFunc(c, "rope/length",   "[rope]",        "Return the length of ROPE", lRopeLengthGet);
	lAddNativeFunc(c, "rope/length!",  "[rope length]", "Set the length of ROPE to LENGTH", lRopeLengthSet);
	lAddNativeFunc(c, "rope/flags",    "[rope]",        "Get the flags of ROPE", lRopeFlagsGet);
	lAddNativeFunc(c, "rope/flags!",   "[rope flags]",  "Set the flags of ROPE", lRopeFlagsSet);
	lAddNativeFunc(c, "rope/a",        "[rope]",        "Get the first being connected by ROPE", lRopeGetA);
	lAddNativeFunc(c, "rope/b",        "[rope]",        "Get the second being connected by ROPE", lRopeGetB);
}
