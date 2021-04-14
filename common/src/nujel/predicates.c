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

#include "predicates.h"

#include "nujel.h"
#include "casting.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>

static int lValCompare(lClosure *c, lVal *v){
	if((v == NULL) || (v->vList.car == NULL) || (v->vList.cdr == NULL)){return 2;}
	lVal *a = lEval(c,v->vList.car);
	v = v->vList.cdr;
	if(v->vList.car == NULL){return 2;}
	lVal *b = lEval(c,v->vList.car);
	if((a == NULL) || (b == NULL)){return 2;}
	lType ct = lTypecast(a->type, b->type);
	switch(ct){
	default:
		return 2;
	case ltInf:
		if((a == NULL) || (b == NULL)){return  2;}
		if((a->type == ltInf) && (b->type == ltInf)){return 0;}
		if(a->type == ltInf){return 1;}
		return -1;
	case ltBool:
	case ltInt:
		a = lnfInt(c,a);
		b = lnfInt(c,b);
		if((a == NULL) || (b == NULL)){return  2;}
		if(b->vInt == a->vInt)        {return  0;}
		else if(a->vInt  < b->vInt)   {return -1;}
		return 1;
	case ltFloat:
		a = lnfFloat(c,a);
		b = lnfFloat(c,b);
		if((a == NULL) || (b == NULL)) {return  2;}
		if(b->vFloat == a->vFloat)     {return  0;}
		else if(a->vFloat  < b->vFloat){return -1;}
		return 1;
	case ltString: {
		const uint alen = lStringLength(&lStr(a));
		const uint blen = lStringLength(&lStr(b));
		const char *ab = lStrBuf(a);
		const char *bb = lStrBuf(b);
		for(uint i=0;i<alen;i++){
			const u8 ac = ab[i];
			const u8 bc = bb[i];
			if(ac == bc){continue;}
			if(ac < bc){return -1;}
			return 1;
		}
		if(alen != blen){
			if(alen < blen){
				return -1;
			}
			return -1;
		}
		return 0; }
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
	return lValBool((t != NULL) && (t->type == ltString));
}

lVal *lnfVecPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltVec));
}

lVal *lnfBoolPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltBool));
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

lVal *lnfLambdaPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltLambda));
}

lVal *lnfNativeFuncPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	return lValBool((t != NULL) && (t->type == ltNativeFunc));
}

void lAddPredicateFuncs(lClosure *c){
	lAddNativeFunc(c,"less? <",           "(a b)","#t if A < B", lnfLess);
	lAddNativeFunc(c,"less-equal? <=",    "(a b)","#t if A <= B",lnfLessEqual);
	lAddNativeFunc(c,"equal? eqv? eq? =", "(a b)","#t if A == B",lnfEqual);
	lAddNativeFunc(c,"greater-equal? >=", "(a b)","#t if A >= B",lnfGreaterEqual);
	lAddNativeFunc(c,"greater? >",        "(a b)","#t if A > B", lnfGreater);
	lAddNativeFunc(c,"zero? z?",          "(a)",  "#t if A == 0",lnfZero);

	lAddNativeFunc(c,"int?",              "(a)","#t if A an int",   lnfIntPred);
	lAddNativeFunc(c,"float?",            "(a)","#t if A a float",  lnfFloatPred);
	lAddNativeFunc(c,"vec?",              "(a)","#t if A a vec",    lnfVecPred);
	lAddNativeFunc(c,"boolean? bool?",    "(a)","#t if A a bool",   lnfBoolPred);
	lAddNativeFunc(c,"nil?",              "(a)","#t if A #nil",     lnfNilPred);
	lAddNativeFunc(c,"inf?",              "(a)","#t if A an #inf",  lnfInfPred);
	lAddNativeFunc(c,"pair?",             "(a)","#t if A a pair",   lnfPairPred);
	lAddNativeFunc(c,"string?",           "(a)","#t if A a string", lnfStringPred);
	lAddNativeFunc(c,"lambda?",           "(a)","#t if A a lambda", lnfLambdaPred);
	lAddNativeFunc(c,"native?",           "(a)","#t if A a #cfn",   lnfNativeFuncPred);
}
