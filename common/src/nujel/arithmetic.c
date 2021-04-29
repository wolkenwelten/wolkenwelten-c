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

#include "arithmetic.h"
#include "nujel.h"
#include "casting.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lVal *lnfAddV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		lVecV(t->vCdr) = vecAdd(lVecV(t->vCdr),lVecV(lCar(vv)->vCdr));
	}
	return t;
}
static lVal *lnfAddF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vFloat += lCar(vv)->vFloat;
	}
	return t;
}
static lVal *lnfAddI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vInt += lCar(vv)->vInt;
	}
	return t;
}
lVal *lnfAdd(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lEvalCastApply(lnfAdd,c,v);
}


static lVal *lnfSubV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		lVecV(t->vCdr) = vecSub(lVecV(t->vCdr),lVecV(lCar(vv)->vCdr));
	}
	return t;
}
static lVal *lnfSubF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vFloat -= lCar(vv)->vFloat;
	}
	return t;
}
static lVal *lnfSubI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vInt -= lCar(vv)->vInt;
	}
	return t;
}
lVal *lnfSub(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	if((v->type == ltPair) && (lCar(v) != NULL) && (lCdr(v) == NULL)){
		v = lCons(lValInt(0),v);
	}
	lEvalCastApply(lnfSub,c,v);
}

static lVal *lnfMulV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		lVecV(t->vCdr) = vecMul(lVecV(t->vCdr),lVecV(lCar(vv)->vCdr));
	}
	return t;
}
static lVal *lnfMulF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vFloat *= lCar(vv)->vFloat; }
	return t;
}
static lVal *lnfMulI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt *= lCar(vv)->vInt; }
	return t;
}
lVal *lnfMul(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(1);}
	lEvalCastApply(lnfMul, c , v);
}


static lVal *lnfDivV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		lVecV(t->vCdr) = vecDiv(lVecV(t->vCdr),lVecV(lCar(vv)->vCdr));
	}
	return t;
}
static lVal *lnfDivF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		const float cv = lCar(vv)->vFloat;
		if(cv == 0){return lValInf();}
		t->vFloat /= cv;
	}
	return t;
}
static lVal *lnfDivI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		if(lCar(vv)->vInt == 0){return lValInf();}
		t->vInt /= lCar(vv)->vInt;
	}
	return t;
}
lVal *lnfDiv(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(1);}
	lEvalCastApply(lnfDiv, c, v);
}



static lVal *lnfModV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		lVecV(t->vCdr) = vecMod(lVecV(t->vCdr),lVecV(lCar(vv)->vCdr));
	}
	return t;
}
static lVal *lnfModF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vFloat = fmodf(t->vFloat,lCar(vv)->vFloat);
	}
	return t;
}
static lVal *lnfModI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt %= lCar(vv)->vInt; }
	return t;
}
lVal *lnfMod(lClosure *c, lVal *v){
	lEvalCastApply(lnfMod, c, v);
}

lVal *lnfAbs(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(fabsf(t->vFloat));
	case ltInt:
		return lValInt(abs(t->vInt));
	case ltVec:
		return lValVec(vecAbs(lVecV(t->vCdr)));
	}
}

lVal *lnfSqrt(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(sqrtf(t->vFloat));
	case ltInt:
		return lValFloat(sqrtf(t->vInt));
	case ltVec:
		return lValVec(vecSqrt(lVecV(t->vCdr)));
	}
}

lVal *lnfCeil(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(ceilf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecCeil(lVecV(t->vCdr)));
	}
}

lVal *lnfFloor(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(floorf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecFloor(lVecV(t->vCdr)));
	}
}

lVal *lnfRound(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(roundf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecRound(lVecV(t->vCdr)));
	}
}

lVal *lnfSin(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return lValFloat(sinf(t->vFloat));
	}
}

lVal *lnfCos(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return lValFloat(cosf(t->vFloat));
	}
}

lVal *lnfTan(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return lValFloat(tanf(t->vFloat));
	}
}


lVal *lnfPow(lClosure *c, lVal *v){
	if(lCdr(v) == NULL){return lValInt(0);}
	v = lEvalCastNumeric(c,v);
	if(lCdr(v) == NULL){return lValInt(0);}
	lVal *t = lCar(v);
	if(t == NULL){return lValInt(0);}
	lVal *u = lCadr(v);
	if(u == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(powf(t->vFloat,u->vFloat));
	case ltInt:
		return lValFloat(powf(t->vInt,u->vInt));
	case ltVec:
		return lValVec(vecPow(lVecV(t->vCdr),lVecV(u->vCdr)));
	}
}

lVal *lnfVX(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastSpecific(c,v,ltVec));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(lVecV(t->vCdr).x);
	}
}

lVal *lnfVY(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastSpecific(c,v,ltVec));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(lVecV(t->vCdr).y);
	}
}

lVal *lnfVZ(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastSpecific(c,v,ltVec));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(lVecV(t->vCdr).z);
	}
}

lVal *infixFunctions[32];
int infixFunctionCount = 0;

void lAddInfix(lVal *v){
	infixFunctions[infixFunctionCount++] = v;
}

void lAddArithmeticFuncs(lClosure *c){
	lAddInfix(lAddNativeFunc(c,"mod %",  "[...args]","Modulo",        lnfMod));
	lAddInfix(lAddNativeFunc(c,"div /",  "[...args]","Division",      lnfDiv));
	lAddInfix(lAddNativeFunc(c,"mul *",  "[...args]","Multiplication",lnfMul));
	lAddInfix(lAddNativeFunc(c,"sub -",  "[...args]","Substraction",  lnfSub));
	lAddInfix(lAddNativeFunc(c,"add +",  "[...args]","Addition",      lnfAdd));

	lAddNativeFunc(c,"abs","[a]",  "Return the absolute value of a",   lnfAbs);
	lAddInfix(lAddNativeFunc(c,"pow","[a b]","Return a raised to the power of b",lnfPow));
	lAddNativeFunc(c,"sqrt","[a]", "Return the squareroot of a",       lnfSqrt);
	lAddNativeFunc(c,"floor","[a]","Round a down",                     lnfFloor);
	lAddNativeFunc(c,"ceil","[a]", "Round a up",                       lnfCeil);
	lAddNativeFunc(c,"round","[a]","Round a",                          lnfRound);
	lAddNativeFunc(c,"sin","[a]",  "Sin A",                            lnfSin);
	lAddNativeFunc(c,"cos","[a]",  "Cos A",                            lnfCos);
	lAddNativeFunc(c,"tan","[a]",  "Tan A",                            lnfTan);

	lAddNativeFunc(c,"vx","[vec]","Return x part of VEC",lnfVX);
	lAddNativeFunc(c,"vy","[vec]","Return y part of VEC",lnfVY);
	lAddNativeFunc(c,"vz","[vec]","Return z part of VEC",lnfVZ);

	lDefineVal(c,"π",  lConst(lValFloat(PI)));
	lDefineVal(c,"PI", lConst(lValFloat(PI)));
}

lVal *lnfInfix (lClosure *c, lVal *v){
	lVal *l = NULL, *start = NULL;
	for(lVal *cur=v;cur != NULL;cur=lCdr(cur)){
		lVal *cv = lEval(c,lCar(cur));
		lVal *new = lCons(cv,NULL);
		if(l == NULL){
			l = start = new;
		}else{
			l->vList.cdr = new;
			l = new;
		}
	}
	for(int i=0;i<infixFunctionCount;i++){
		lVal *func;
		for(lVal *cur=start;cur != NULL;cur=lCdr(cur)){
			tryAgain: func = lCadr(cur);
			if((func == NULL) || (func->vCdr == 0))  {break;}
			if(func->vCdr != infixFunctions[i]->vCdr){continue;}
			if(func->type != infixFunctions[i]->type){continue;}
			lVal *args = cur;
			lVal *tmp = args->vList.car;
			args->vList.car = lCadr(args);
			lCdr(args)->vList.car = tmp;
			tmp = lCddr(args)->vList.cdr;
			lCddr(args)->vList.cdr = NULL;
			args->vList.car = lEval(c,args);
			args->vList.cdr = tmp;
			goto tryAgain;
		}
	}
	return start;
}
