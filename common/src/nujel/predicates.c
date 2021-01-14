#include "predicates.h"

#include "nujel.h"
#include "arithmetic.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>

static int lValCompare(lClosure *c, lVal *v){
	if(v->vList.car == NULL){return 2;}
	if(v->vList.cdr == NULL){return 2;}
	lVal *a = lEval(c,v->vList.car);
	v = v->vList.cdr;
	if(v->vList.car == NULL){return 2;}
	lVal *b = lEval(c,v->vList.car);
	if(a       == NULL){return 2;}
	if(b       == NULL){return 2;}
	lType ct = lTypecast(a->type, b->type);
	switch(ct){
	default:
		printf("default typecast: %i\n",ct);
		return 2;
	case ltBool:
	case ltInt:
		a = lnfInt(c,a);
		b = lnfInt(c,b);
		if(b->vInt == a->vInt)     {return  0;}
		else if(a->vInt  < b->vInt){return -1;}
		return 1;
	case ltFloat:
		a = lnfFloat(c,a);
		b = lnfFloat(c,b);
		if(b->vFloat == a->vFloat)     {return  0;}
		else if(a->vFloat  < b->vFloat){return -1;}
		return 1;
	}
}

lVal *lnfLess(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	if(cmp == 2){return lValBool(false);}
	return lValBool(cmp < 0);
}

lVal *lnfEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	if(cmp == 2){return lValBool(false);}
	return lValBool(cmp == 0);
}

lVal *lnfLessEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	if(cmp == 2){return lValBool(false);}
	return lValBool(cmp <= 0);
}

lVal *lnfGreater(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	if(cmp == 2){return lValBool(false);}
	return lValBool(cmp > 0);
}

lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	if(cmp == 2){return lValBool(false);}
	return lValBool(cmp >= 0);
}

lVal *lnfZero(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,lCons(lValInt(0),v));
	if(cmp == 2){return lValBool(false);}
	return lValBool(cmp == 0);
}

lVal *lnfIntPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	return lValBool(t->type == ltInt);
}

lVal *lnfFloatPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	return lValBool(t->type == ltFloat);
}

lVal *lnfNumberPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	switch(t->type){
	case ltInt:
	case ltFloat:
	case ltVec:
		return lValBool(true);
	default:
		return lValBool(false);
	}
}

lVal *lnfStringPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return lValBool(false);}
	return lValBool((t->type == ltString) || (t->type == ltCString));
}

lVal *lnfEmptyPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(true);}
	switch(t->type){
	default: return lValBool(true);
	case ltNil: return lValBool(false);
	case ltString:
		if(t->vString == NULL){return lValBool(true);}
		for(const char *str = t->vString->data;str < t->vString->bufEnd;str++){if(!isspace(*str)){return lValBool(false);}}
		return lValBool(true);
	}
}

lVal *lnfPosPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	if(t->type != ltInt){return lValNil();}
	return lValBool(t->vInt >= 0);
}

lVal *lnfNegPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	if(t->type != ltInt){return lValNil();}
	return lValBool(t->vInt < 0);
}

lVal *lnfVecPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	return lValBool(t->type == ltVec);
}

lVal *lnfNilPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	return lValBool(t->type == ltNil);
}

lVal *lnfInfPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValBool(false);}
	return lValBool(t->type == ltInf);
}
