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
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){return 2;}
	lVal *a = lEval(c,lCar(v));
	v = lCdr(v);
	if(lCar(v) == NULL){return 2;}
	lVal *b = lEval(c,lCar(v));
	if((a == NULL) || (b == NULL)){return 2;}
	lType ct = lTypecast(a->type, b->type);
	switch(ct){
	default:
		return 2;
	case ltSymbol:
	case ltLambda:
	case ltNativeFunc:
		if(a->type != b->type){return -1;}
		if(b->vCdr != a->vCdr){return -1;}
		return 0;
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

lVal *lnfNilPred(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	return lValBool(t == NULL);
}

void lAddPredicateFuncs(lClosure *c){
	lAddNativeFunc(c,"less? <",           "(a b)","#t if A < B",  lnfLess);
	lAddNativeFunc(c,"less-equal? <=",    "(a b)","#t if A <= B", lnfLessEqual);
	lAddNativeFunc(c,"equal? eqv? eq? =", "(a b)","#t if A == B", lnfEqual);
	lAddNativeFunc(c,"greater-equal? >=", "(a b)","#t if A >= B", lnfGreaterEqual);
	lAddNativeFunc(c,"greater? >",        "(a b)","#t if A > B",  lnfGreater);
	lAddNativeFunc(c,"nil?",              "(a)","#t if A #nil",   lnfNilPred);
}
