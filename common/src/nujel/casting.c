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

#include "casting.h"

#include "nujel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

lVal *lnfInf(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInf();
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
		return lValInt(lVecV(t->vCdr).x);
	case ltString:
		if(t->vCdr == 0){return lValInt(0);}
		return lValInt(atoi(lStrData(t)));
	case ltPair:
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
		return lValFloat(lVecV(t->vCdr).x);
	case ltString:
		if(t->vCdr == 0){return lValFloat(0);}
		return lValFloat(atof(lStrData(t)));
	case ltPair:
		return lnfFloat(c,v->vList.car);
	}
}

lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0);
	if(v == NULL){return lValVec(nv);}
	if(v->type == ltVec){return v;}
	if(v->type != ltPair){
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

lVal *lnfBool(lClosure *c, lVal *v){
	lVal *a = lEval(c,v);
	if(a == NULL)            {return lValBool(false);}
	if(a->type == ltPair)    {a = a->vList.car;}
	if(a == NULL)            {return lValBool(false);}
	if(a->type == ltPair)    {a = lEval(c,a);}
	if(a == NULL)            {return lValBool(false);}
	if(a->type == ltBool)    {return a;}
	if(a->type == ltInt)     {return lValBool(a->vInt != 0);}
	return lValBool(true);
}

lVal *lnfString(lClosure *c, lVal *t){
	char tmpStringBuf[32];
	char *buf = tmpStringBuf;
	int len = 0;
	if(t == NULL){return lValString("");}
	(void)c;

	switch(t->type){
	default: break;
	case ltFloat: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%f",t->vFloat);
		len += clen;
		buf += clen;
		break; }
	case ltInt: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%i",t->vInt);
		len += clen;
		buf += clen;
		break; }
	case ltString:
		return t;
	}

	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringNew(tmpStringBuf, len);
	return ret;
}

void lAddCastingFuncs(lClosure *c){
	lAddNativeFunc(c,"bool", "(a)","Casts a to bool",  lnfBool);
	lAddNativeFunc(c,"int",  "(a)","Casts a to int",   lnfInt);
	lAddNativeFunc(c,"float","(a)","Casts a to float", lnfFloat);
	lAddNativeFunc(c,"vec",  "(a)","Casts a to vec",   lnfVec);
	lAddNativeFunc(c,"str",  "(a)","Casts a to string",lnfCat);
}
