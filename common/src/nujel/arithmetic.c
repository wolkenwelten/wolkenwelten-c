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

#include "arithmetic.h"
#include "nujel.h"
#include "casting.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lVal *lnfAddV(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vVec = vecAdd(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfAddF(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vFloat += vv->vList.car->vFloat; }
	return t;
}
static lVal *lnfAddI(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vInt += vv->vList.car->vInt; }
	return t;
}
lVal *lnfAdd(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lEvalCastApply(lnfAdd,c,v);
}


static lVal *lnfSubV(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vVec = vecSub(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfSubF(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vFloat -= vv->vList.car->vFloat; }
	return t;
}
static lVal *lnfSubI(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vInt -= vv->vList.car->vInt; }
	return t;
}
lVal *lnfSub(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	if((v->type == ltPair) && (v->vList.car != NULL) && (v->vList.cdr == NULL)){
		v = lCons(lValInt(0),v);
	}
	lEvalCastApply(lnfSub,c,v);
}

static lVal *lnfMulV(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vVec = vecMul(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfMulF(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vFloat *= vv->vList.car->vFloat; }
	return t;
}
static lVal *lnfMulI(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vInt *= vv->vList.car->vInt; }
	return t;
}
lVal *lnfMul(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(1);}
	lEvalCastApply(lnfMul, c , v);
}


static lVal *lnfDivV(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vVec = vecDiv(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfDivF(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){
		if(vv->vList.car->vFloat == 0){return lValInf();}
		t->vFloat /= vv->vList.car->vFloat;
	}
	return t;
}
static lVal *lnfDivI(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){
		if(vv->vList.car->vInt == 0){return lValInf();}
		t->vInt /= vv->vList.car->vInt;
	}
	return t;
}
lVal *lnfDiv(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(1);}
	lEvalCastApply(lnfDiv, c, v);
}



static lVal *lnfModV(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vVec = vecMod(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfModF(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vFloat = fmodf(t->vFloat,vv->vList.car->vFloat); }
	return t;
}
static lVal *lnfModI(lVal *t, lVal *v){
	forEach(vv,v->vList.cdr){ t->vInt %= vv->vList.car->vInt; }
	return t;
}
lVal *lnfMod(lClosure *c, lVal *v){
	lEvalCastApply(lnfMod, c, v);
}

lVal *lnfAbs(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastNumeric(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(fabsf(t->vFloat));
	case ltInt:
		return lValInt(abs(t->vInt));
	case ltVec:
		return lValVec(vecAbs(t->vVec));
	}
}

lVal *lnfSqrt(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastNumeric(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(sqrtf(t->vFloat));
	case ltInt:
		return lValFloat(sqrtf(t->vInt));
	case ltVec:
		return lValVec(vecSqrt(t->vVec));
	}
}

lVal *lnfCeil(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(ceilf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecCeil(t->vVec));
	}
}

lVal *lnfFloor(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(floorf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecFloor(t->vVec));
	}
}

lVal *lnfRound(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(roundf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecRound(t->vVec));
	}
}

lVal *lnfPow(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair) || (v->vList.cdr == NULL)){return lValInt(0);}
	v = lEvalCastNumeric(c,v);
	if((v == NULL) || (v->type != ltPair) || (v->vList.cdr == NULL)){return lValInt(0);}
	lVal *t = lCarOrV(v);
	if(t == NULL){return lValInt(0);}
	lVal *u = lCarOrV(v->vList.cdr);
	if(u == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(powf(t->vFloat,u->vFloat));
	case ltInt:
		return lValFloat(powf(t->vInt,u->vInt));
	case ltVec:
		return lValVec(vecPow(t->vVec,u->vVec));
	}
}

lVal *lnfVX(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastSpecific(c,v,ltVec));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(t->vVec.x);
	}
}

lVal *lnfVY(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastSpecific(c,v,ltVec));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(t->vVec.y);
	}
}

lVal *lnfVZ(lClosure *c, lVal *v){
	lVal *t = lCarOrV(lEvalCastSpecific(c,v,ltVec));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(t->vVec.z);
	}
}
