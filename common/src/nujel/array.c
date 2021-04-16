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
	lVal *arr = lEval(c,lCar(v));
	if((arr == NULL) || (arr->type != ltArray)){return lValInt(0);}
	return lValInt(lArrLength(arr));
}

lVal *lnfArrRef(lClosure *c, lVal *v){
	lVal *arr = lEval(c,lCar(v));
	if((arr == NULL) || (arr->type != ltArray) || (v == NULL)){return NULL;}
	v = lCdr(v);
	lVal *t = lEval(c, lCar(v));
	if(t == NULL){return arr;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	const lVal *lkey = lnfInt(c,t);
	if(lkey == NULL){return NULL;}
	const int key = lkey->vInt;
	if((key < 0) || (key >= lArrLength(arr))){return NULL;}
	const int val = lArrData(arr)[key];
	return lValD(val);
}

lVal *lnfArrSet(lClosure *c, lVal *v){
	lVal *arr = lEval(c,lCar(v));
	if((arr == NULL) || (arr->type != ltArray) || (v == NULL)){return NULL;}
	v = lCdr(v);
	lVal *t = lEval(c, lCar(v));
	if(t == NULL){return NULL;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	const lVal *lkey = lnfInt(c,t);
	if(lkey == NULL){return NULL;}
	int key = lkey->vInt;
	if((key < 0) || (key >= lArrLength(arr))){return NULL;}
	v = lCdr(v);
	forEach(cur,v){
		lVal *cv = lEval(c,lCar(cur));
		lArrData(arr)[key++] = cv == NULL ? 0 : lValI(cv);
		if(key >= lArrLength(arr)){return NULL;}
	}
	int val = lArrData(arr)[key];
	return lValD(val);
}

lVal *lnfArrNew(lClosure *c, lVal *v){
	lVal *t = lnfInt(c,v);
	if((t == NULL) || (t->type != ltInt)){return NULL;}
	lVal *r = lValAlloc();
	r->type = ltArray;
	r->vCdr = lArrayAlloc();
	lArrLength(r) = t->vInt;
	lArrData(r) = malloc(t->vInt * sizeof(*lArrData(r)));
	if(lArrData(r) == NULL){
		lArrLength(r) = 0;
		return NULL;
	}
	memset(lArrData(r),0,lArrLength(r) * sizeof(*lArrData(r)));
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
	arr->data = malloc(length * sizeof(*arr->data));
	if(arr->data == NULL){
		arr->length = 0;
		lValFree(r);
		return NULL;
	}
	memset(lArrData(r),0,lArrLength(r) * sizeof(*arr->data));
	int key = 0;
	forEach(cur,vals){
		lArrData(r)[key++] = lValI(lCar(cur));
	}
	return r;
}

lVal *lnfArrPred(lClosure *c, lVal *v){
	lVal *arr = lEval(c,lCar(v));
	if((arr == NULL) || (arr->type != ltArray)){return lValBool(false);}
	return lValBool(true);
}

void lAddArrayFuncs(lClosure *c){
	lAddNativeFunc(c,"arr-length", "(array)",    "Return length of ARRAY",                          lnfArrLength);
	lAddNativeFunc(c,"arr-ref",    "(array index)",  "Return value of ARRAY at position INDEX",     lnfArrRef);
	lAddNativeFunc(c,"arr-set!",   "(array index &...values)","Set ARRAY at INDEX to &...VALUES",   lnfArrSet);
	lAddNativeFunc(c,"arr-new",    "(size)",     "Allocate a new array of SIZE",                    lnfArrNew);
	lAddNativeFunc(c,"arr",        "(...args)",  "Create a new array from ...ARGS",                 lnfArr);
	lAddNativeFunc(c,"array? arr?","(val)",     "Return #t if VAL is an array",                     lnfArrPred);
}
