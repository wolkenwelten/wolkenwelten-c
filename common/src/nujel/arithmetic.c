#include "arithmetic.h"
#include "nujel.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lVal *lnfAddV(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vVec = vecAdd(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfAddF(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vFloat += vv->vList.car->vFloat; }
	return t;
}
static lVal *lnfAddI(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vInt += vv->vList.car->vInt; }
	return t;
}
lVal *lnfAdd(lClosure *c, lVal *v){
	lEvalCastApply(lnfAdd,c,v);
}


static lVal *lnfSubV(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vVec = vecSub(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfSubF(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vFloat -= vv->vList.car->vFloat; }
	return t;
}
static lVal *lnfSubI(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vInt -= vv->vList.car->vInt; }
	return t;
}
lVal *lnfSub(lClosure *c, lVal *v){
	if((v->type == ltList) && (v->vList.car != NULL) && (v->vList.cdr == NULL)){
		v = lCons(lValInt(0),v);
	}
	lEvalCastApply(lnfSub,c,v);
}

static lVal *lnfMulV(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vVec = vecMul(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfMulF(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vFloat *= vv->vList.car->vFloat; }
	return t;
}
static lVal *lnfMulI(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vInt *= vv->vList.car->vInt; }
	return t;
}
lVal *lnfMul(lClosure *c, lVal *v){
	lEvalCastApply(lnfMul, c , v);
}


static lVal *lnfDivV(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vVec = vecDiv(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfDivF(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){
		if(vv->vList.car->vFloat == 0){return lValInf();}
		t->vFloat /= vv->vList.car->vFloat;
	}
	return t;
}
static lVal *lnfDivI(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){
		if(vv->vList.car->vInt == 0){return lValInf();}
		t->vInt /= vv->vList.car->vInt;
	}
	return t;
}
lVal *lnfDiv(lClosure *c, lVal *v){
	lEvalCastApply(lnfDiv, c, v);
}



static lVal *lnfModV(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vVec = vecMod(t->vVec,vv->vList.car->vVec); }
	return t;
}
static lVal *lnfModF(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vFloat = fmodf(t->vFloat,vv->vList.car->vFloat); }
	return t;
}
static lVal *lnfModI(lVal *v){
	lVal *t = lValDup(v->vList.car);
	forEach(vv,v->vList.cdr){ t->vInt %= vv->vList.car->vInt; }
	return t;
}
lVal *lnfMod(lClosure *c, lVal *v){
	lEvalCastApply(lnfMod, c, v);
}



lVal *lnfInt(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default: return lValInt(0);
	case ltBool:
		return lValInt(t->vBool ? 1 : 0);
	case ltInt:
		return t;
	case ltFloat:
		return lValInt(t->vFloat);
	case ltVec:
		return lValInt(t->vVec.x);
	case ltCString:
		if(t->vCString == NULL){return lValInt(0);}
		return lValInt(atoi(t->vCString->data));
	case ltString:
		if(t->vString == NULL){return lValInt(0);}
		return lValInt(atoi(t->vString->data));
	case ltList:
		return lnfInt(c,v->vList.car);
	}
}

lVal *lnfFloat(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default: return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lValFloat(t->vInt);
	case ltVec:
		return lValFloat(t->vVec.x);
	case ltCString:
		if(t->vCString == NULL){return lValFloat(0);}
		return lValFloat(atof(t->vCString->data));
	case ltString:
		if(t->vString == NULL){return lValFloat(0);}
		return lValFloat(atof(t->vString->data));
	case ltList:
		return lnfFloat(c,v->vList.car);
	}
}


lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0);
	if(v == NULL){return lValVec(nv);}
	if(v->type == ltVec){return v;}
	if(v->type != ltList){
		v = lnfFloat(c,v);
		return lValVec(vecNew(v->vFloat,v->vFloat,v->vFloat));
	}
	int i = 0;
	forEach(cv,v){
		lVal *t = lEval(c,cv->vList.car);
		if(t == NULL){break;}
		if(t->type == ltVec){return t;}
		t = lnfFloat(c,t);
		if(t == NULL){break;}
		for(int ii=i;ii<3;ii++){
			nv.v[ii] = t->vFloat;
		}
		if(++i >= 3){break;}
	}
	return lValVec(nv);
}

lVal *lnfInf(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInf();
}

lVal *lnfAbs(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValInt(0);}
	if((t->type != ltInt) && (t->type != ltFloat) && (t->type != ltVec)){
		t = lnfFloat(c,t);
	}
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
	if(v == NULL){return lValFloat(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
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
	if(v == NULL){return lValFloat(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
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
	if(v == NULL){return lValFloat(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
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
