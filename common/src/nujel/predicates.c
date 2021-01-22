#include "predicates.h"

#include "nujel.h"
#include "casting.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>

static int lValCompare(lClosure *c, lVal *v){
	if((v->vList.car == NULL) || (v->vList.cdr == NULL)){return 2;}
	lVal *a = lEval(c,v->vList.car);
	v = v->vList.cdr;
	if(v->vList.car == NULL){return 2;}
	lVal *b = lEval(c,v->vList.car);
	if((a == NULL) || (b == NULL)){return 2;}
	lType ct = lTypecast(a->type, b->type);
	switch(ct){
	default:
		return 2;
	case ltBool:
	case ltInt:
		a = lnfInt(c,a);
		b = lnfInt(c,b);
		if((a == NULL) || (b == NULL)){return 2;}
		if(b->vInt == a->vInt)     {return  0;}
		else if(a->vInt  < b->vInt){return -1;}
		return 1;
	case ltFloat:
		a = lnfFloat(c,a);
		b = lnfFloat(c,b);
		if((a == NULL) || (b == NULL)){return 2;}
		if(b->vFloat == a->vFloat)     {return  0;}
		else if(a->vFloat  < b->vFloat){return -1;}
		return 1;
	case ltString:
		for(int i=0;i<a->vString->len;i++){
			const u8 ac = a->vString->buf[i];
			const u8 bc = b->vString->buf[i];
			if(ac == bc){continue;}
			if(ac < bc){return -1;}
			return 1;
		}
		if(a->vString->len != b->vString->len){
			if(a->vString->len < b->vString->len){
				return -1;
			}
			return -1;
		}
		return 0;
	}
}

lVal *lnfLess(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp < 0);
}

lVal *lnfEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp == 0);
}

lVal *lnfLessEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp <= 0);
}

lVal *lnfGreater(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp > 0);
}

lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp >= 0);
}

lVal *lnfZero(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,lCons(lValInt(0),v));
	return lValBool(cmp == 2 ? false : cmp == 0);
}

lVal *lnfIntPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool(t == NULL ? false : t->type == ltInt);
}

lVal *lnfFloatPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool(t == NULL ? false : t->type == ltFloat);
}

lVal *lnfStringPred(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && ((t->type == ltString) || (t->type == ltCString)));
}

lVal *lnfVecPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltVec));
}

lVal *lnfNilPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool(t == NULL);
}

lVal *lnfInfPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltInf));
}

lVal *lnfPairPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltPair));
}
