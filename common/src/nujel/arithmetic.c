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
	if((v->type == ltList) && (v->vList.car != NULL) && (v->vList.cdr == NULL)){
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
	lVal *t = lEvalCastNumeric(c,v);
	if(t == NULL){return lValFloat(0);}
	if(t->type == ltList){t = t->vList.car;}
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

lVal *lnfVX(lClosure *c, lVal *v){
	lVal *t = lEvalCastSpecific(c,v,ltVec);
	if(t == NULL){return lValFloat(0);}
	if(t->type == ltList){t = t->vList.car;}
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
	lVal *t = lEvalCastSpecific(c,v,ltVec);
	if(t == NULL){return lValFloat(0);}
	if(t->type == ltList){t = t->vList.car;}
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
	lVal *t = lEvalCastSpecific(c,v,ltVec);
	if(t == NULL){return lValFloat(0);}
	if(t->type == ltList){t = t->vList.car;}
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
