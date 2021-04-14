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

#include "array.h"

#include "casting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lVal *lnfArrLength(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *arr = lEval(c,lCarOrV(v));
	if((arr == NULL) || (arr->type != ltArray)){return NULL;}
	return lValInt(lArrLength(arr));
}

lVal *lnfArrRS(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *arr = lEval(c,lCarOrV(v));
	if((arr == NULL) || (arr->type != ltArray)){return NULL;}
	v = v->vList.cdr;
	if(v == NULL){return arr;}
	lVal *t = lEval(c, lCarOrV(v));
	if(t == NULL){return NULL;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	const lVal *lkey = lnfInt(c,t);
	if(lkey == NULL){return NULL;}
	int key = lkey->vInt;
	if((key < 0) || (key >= lArrLength(arr))){return NULL;}
	v = v->vList.cdr;
	forEach(cur,v){
		lArrData(arr)[key++] = lEval(c,lCarOrV(cur));
		if(key >= lArrLength(arr)){return NULL;}
	}
	return lArrData(arr)[key];
}

lVal *lnfArrNew(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *t = lnfInt(c,v);
	if((t == NULL) || (t->type != ltInt)){return NULL;}
	lVal *r = lValAlloc();
	r->type = ltArray;
	r->vCdr = lArrayAlloc();
	lArrLength(r) = t->vInt;
	lArrData(r) = malloc(t->vInt * sizeof(lVal *));
	if(lArrData(r) == NULL){
		lArrLength(r) = 0;
		return NULL;
	}
	memset(lArrData(r),0,lArrLength(r) * sizeof(lVal *));
	return r;
}

lVal *lnfArr(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *vals = lApply(c,v,lEval);;
	int length = lListLength(vals);
	lVal *r = lValAlloc();
	r->type = ltArray;
	r->vCdr = lArrayAlloc();
	if(r->vCdr == 0){return NULL;}
	lArray *arr = &lArr(r);
	arr->length = length;
	arr->data = malloc(length * sizeof(lVal *));
	if(arr->data == NULL){
		arr->length = 0;
		lValFree(r);
		return NULL;
	}
	memset(lArrData(r),0,lArrLength(r) * sizeof(lVal *));
	int key = 0;
	forEach(cur,vals){
		lArrData(r)[key++] = cur->vList.car;
	}
	return r;
}

lVal *lnfArrPred(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return lValBool(false);}
	lVal *arr = lEval(c,lCarOrV(v));
	if((arr == NULL) || (arr->type != ltArray)){return lValBool(false);}
	return lValBool(true);
}

void lAddArrayFuncs(lClosure *c){
	lAddNativeFunc(c,"arr-length","(a)",        "Returns length of array a",                 lnfArrLength);
	lAddNativeFunc(c,"arr-ref",   "(a i)",      "Returns value of array a at position i",    lnfArrRS);
	lAddNativeFunc(c,"arr-set!",  "(a i &...v)","Sets array valus at position i to v",       lnfArrRS);
	lAddNativeFunc(c,"arr-new",   "(l)",        "Allocates a new array of size l",           lnfArrNew);
	lAddNativeFunc(c,"arr",       "(...args)",  "Creates a new array from its argument list",lnfArr);
	lAddNativeFunc(c,"array? arr?","(val)",     "Return #t if VAL is an array",lnfArrPred);
}
