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
	rope *r = ropeNew(a, b, 0);
	r->length = 32;
	const int ID = ropeGetID(r);
	return lValInt(ID);
}

static lVal *lRopeLengthGet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	return lValInt(r->length);
}

static lVal *lRopeFlagsGet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	return lValInt(r->flags);
}

static lVal *lRopeLengthSet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	r->length = requireInt(c, lCadr(v));
	return NULL;
}

static lVal *lRopeFlagsSet(lClosure *c, lVal *v){
	rope *r = requireRope(c, lCar(v));
	if(r == NULL){
		lExceptionThrowValClo("invalid-reference", "Can't find a rope for:", lCar(v), c);
	}
	r->flags = requireInt(c, lCadr(v));
	return NULL;
}

void lOperatorsRope(lClosure *c){
	lAddNativeFunc(c,"rope/new*",    "[a b]",         "Create a new rope connecting beings A and B", lRopeNew);
	lAddNativeFunc(c,"rope/length",  "[rope]",        "Return the length of ROPE", lRopeLengthGet);
	lAddNativeFunc(c,"rope/length!", "[rope length]", "Set the length of ROPE to LENGTH", lRopeLengthSet);
	lAddNativeFunc(c,"rope/flags",   "[rope]",        "Get the flags of ROPE", lRopeFlagsGet);
	lAddNativeFunc(c,"rope/flags!",  "[rope flags]",  "Set the flags of ROPE", lRopeFlagsSet);
}
