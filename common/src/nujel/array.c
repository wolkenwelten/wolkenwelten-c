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
	return lValInt(arr->vArr->length);
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
	if((key < 0) || (key >= arr->vArr->length)){return NULL;}
	v = v->vList.cdr;
	forEach(cur,v){
		arr->vArr->data[key++] = lEval(c,lCarOrV(cur));
		if(key >= arr->vArr->length){return NULL;}
	}
	return arr->vArr->data[key];
}

lVal *lnfArrNew(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *t = lCarOrV(lCast(c,lEval(c, v),ltInt));
	if((t == NULL) || (t->type != ltInt)){return NULL;}
	lVal *r = lValAlloc();
	r->type = ltArray;
	r->vArr = lArrayAlloc();
	r->vArr->length = t->vInt;
	r->vArr->data = malloc(r->vArr->length * sizeof(lVal *));
	if(r->vArr->data == NULL){
		r->vArr->length = 0;
		return NULL;
	}
	memset(r->vArr->data,0,r->vArr->length * sizeof(lVal *));
	return r;
}

lVal *lnfArr(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	int length = lListLength(v);
	lVal *r = lValAlloc();
	r->type = ltArray;
	r->vArr = lArrayAlloc();
	r->vArr->length = length;
	r->vArr->data = malloc(r->vArr->length * sizeof(lVal *));
	if(r->vArr->data == NULL){
		r->vArr->length = 0;
		return NULL;
	}
	memset(r->vArr->data,0,r->vArr->length * sizeof(lVal *));
	int key = 0;
	forEach(cur,v){
		r->vArr->data[key++] = lEval(c,lCarOrV(cur));
	}
	return r;
}

void lAddArrayFuncs(lClosure *c){
	lAddNativeFunc(c,"arr-length","(a)",        "Returns length of array a",                 lnfArrLength);
	lAddNativeFunc(c,"arr-ref",   "(a i)",      "Returns value of array a at position i",    lnfArrRS);
	lAddNativeFunc(c,"arr-set!",  "(a i &...v)","Sets array valus at position i to v",       lnfArrRS);
	lAddNativeFunc(c,"arr-new",   "(l)",        "Allocates a new array of size l",           lnfArrNew);
	lAddNativeFunc(c,"arr",       "(...args)",  "Creates a new array from its argument list",lnfArr);
}
