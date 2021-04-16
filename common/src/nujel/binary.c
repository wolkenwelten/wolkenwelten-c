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

#include "binary.h"
#include "casting.h"

static lVal *lnfLogAndI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt &= lCar(vv)->vInt; }
	return t;
}
lVal *lnfLogAnd (lClosure *c, lVal *v){
	lEvalCastIApply(lnfLogAndI,c,v);
}

static lVal *lnfLogIorI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt |= lCar(vv)->vInt; }
	return t;
}
lVal *lnfLogIor (lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lEvalCastIApply(lnfLogIorI,c,v);
}

static lVal *lnfLogXorI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt ^= lCar(vv)->vInt; }
	return t;
}
lVal *lnfLogXor (lClosure *c, lVal *v){
	lEvalCastIApply(lnfLogXorI,c,v);
}

lVal *lnfLogNot (lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lEvalCastSpecific(c,v,ltInt);
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);}
	return lValInt(~lCar(t)->vInt);
}

#include <stdio.h>
lVal *lnfAsh(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return lValInt(0);}
	lVal *vals  = lEvalCastSpecific(c,v,ltInt);
	if(vals == NULL){return lValInt(0);}
	lVal *val   = lCar(vals);
	lVal *shift = lCadr(vals);
	if(shift == NULL){return val;}
	const int sv = shift->vInt;
	if(sv > 0){
		return lValInt(val->vInt << shift->vInt);
	}else{
		return lValInt(val->vInt >> -sv);
	}
}

void lAddBinaryFuncs(lClosure *c){
	lAddNativeFunc(c,"logand &","(...args)","And ...ARGS together",                 lnfLogAnd);
	lAddNativeFunc(c,"logior |","(...args)","Or ...ARGS",                           lnfLogIor);
	lAddNativeFunc(c,"logxor ^","(...args)","Xor ...ARGS",                          lnfLogXor);
	lAddNativeFunc(c,"lognot ~","(val)",    "Binary not of VAL",                    lnfLogNot);
	lAddNativeFunc(c,"ash <<",  "(value amount)","Shift VALUE left AMOUNT bits",    lnfAsh);
}
