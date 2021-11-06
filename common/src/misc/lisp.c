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

#include "lisp.h"

#include "../asm/asm.h"
#include "../game/grenade.h"
#include "../game/item.h"
#include "../game/itemDrop.h"
#include "../game/itemType.h"
#include "../game/weather.h"
#include "../misc/colors.h"
#include "../misc/profiling.h"

#include "../../nujel/lib/api.h"
#include "../../nujel/lib/allocation/roots.h"

extern unsigned  int src_tmp_wwlib_nuj_len;
extern unsigned char src_tmp_wwlib_nuj_data[];

#include <ctype.h>
#include <string.h>
#include <stdio.h>

lClosure *clRoot;

lVal *wwlnfMsPerTick(lClosure *c, lVal *v){
	(void)c;
	const int newVal = castToInt(lCar(v),0);
	if(newVal > 0){ msPerTick = newVal; }
	return lValInt(msPerTick);
}

lVal *wwlnfProf(lClosure *c, lVal *v){
	(void)c,(void)v;
	return lValString(profGetReport());
}

lVal *wwlnfProfReset(lClosure *c, lVal *v){
	(void)c,(void)v;
	profReset();
	return lValBool(true);
}

lVal *wwlnfNProf(lClosure *c, lVal *v){
	(void)c,(void)v;
	return lValString(nprofGetReport());
}

lVal *wwlnfNProfReset(lClosure *c, lVal *v){
	(void)c,(void)v;
	nprofReset();
	return lValBool(true);
}

lVal *wwlnfAsmSwitch(lClosure *c, lVal *v){
	(void)c;
	const int newVal = castToInt(lCar(v),-1);
	if(newVal >= 0){asmRoutineSupport = newVal;}
	return lValInt(asmRoutineSupport);
}

lVal *wwlnfExplode(lClosure *c, lVal *v){
	(void)c;
	const vec   pos      = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	const float strength = castToFloat(lCar(v),4.f);     v = lCdr(v);
	const int   style    = castToInt(lCar(v),0);

	if((pos.x < 0.f) || (pos.y < 0.f) || (pos.z < 0.f) || (strength < .1f)){return NULL;}
	explode(pos,strength,style);
	return NULL;
}

static lVal *wwlnfItemDropNew(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	const int id  = castToInt(lCar(v),0);         v = lCdr(v);
	const int amt = castToInt(lCar(v),0);

	if((pos.x < 0.f) || (pos.y < 0.f) || (pos.z < 0.f) || (id <= 0) || (amt <= 0)){return NULL;}
	item itm = itemNew(id,amt);
	itemDropNewP(pos,&itm,-1);
	return NULL;
}

static lVal *wwlnfWVel(lClosure *c, lVal *v){
	(void)c;
	if((v != NULL) && (lCar(v) != NULL) && (lCar(v)->type == ltVec)){
		cloudsSetWind(castToVec(lCar(v),vecZero()));
	}
	return lValVec(windVel);
}

static lVal *wwlnfCDen(lClosure *c, lVal *v){
	(void)c;
	const int cden = castToInt(lCar(v),-1);
	if(cden >= 0){cloudsSetDensity(cden);}
	return lValInt(cloudGDensityMin);
}

static lVal *wwlnfRain(lClosure *c, lVal *v){
	(void)c;
	const int inten = castToInt(lCar(v),-1);
	if(inten >= 0){weatherSetRainDuration(inten);}
	return lValInt(rainIntensity);
}

static lVal *wwlnfColorInterpolate(lClosure *c, lVal *v){
	(void)c;
	const int ca  = castToInt(lCar(v),0); v = lCdr(v);
	const int cb  = castToInt(lCar(v),0); v = lCdr(v);
	const float i = castToFloat(lCar(v),0.f);

	return lValInt(colorInterpolate(ca,cb,i));
}

void lispDefineInt(const char *symbol, int val){
	lDefineVal(clRoot,symbol,lValInt(val));
}

void lispDefineString(const char *symbol, char *str){
	lDefineVal(clRoot,symbol,lValString(str));
}

lClosure *lispCommonRoot(){
	lClosure *c = lClosureNewRoot();

	lAddNativeFunc(c,"mst!",         "(a)",                    "Set ms per tick to s",                                       wwlnfMsPerTick);
	lAddNativeFunc(c,"prof",         "()",                     "Return profiler info",                                       wwlnfProf);
	lAddNativeFunc(c,"prof-reset!",  "()",                     "Reset performance counters",                                 wwlnfProfReset);
	lAddNativeFunc(c,"nprof",        "()",                     "Return network profiler info",                               wwlnfNProf);
	lAddNativeFunc(c,"nprof-reset!", "()",                     "Reset network counters",                                     wwlnfNProfReset);
	lAddNativeFunc(c,"asm-switch!",  "(a)",                    "Switch asm/simd routines",                                   wwlnfAsmSwitch);
	lAddNativeFunc(c,"cloud-thresh!","(&thresh)",              "Set cloud threshold to &THRESH",                             wwlnfCDen);
	lAddNativeFunc(c,"wind-velocity","(&vel)",                 "Set wind velocity to vector &VEL",                           wwlnfWVel);
	lAddNativeFunc(c,"rain-set",     "(&intensity)",           "Set rain rate to a",                                         wwlnfRain);
	lAddNativeFunc(c,"explode",      "(pos &strength &style)", "Create an explosion at POS with &STRENGTH=4.0 and &STYLE=0", wwlnfExplode);
	lAddNativeFunc(c,"item-drop-new","(pos id amount)",        "Create a new item at POS for AMOUNT ID.",                    wwlnfItemDropNew);
	lAddNativeFunc(c,"color-inter",  "(a b i)",                "Interpolate between A and B with 0.0 <= i <= 1.0",          wwlnfColorInterpolate);
	itemTypeLispClosure(c);

	lEval(c,lWrap(lRead((const char *)src_tmp_wwlib_nuj_data)));
	return c;
}

lVal *lispCallFuncI(const char *symbol, int ia){
	lVal *form = lCons(NULL,NULL);
	lRootsValPush(form);
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValInt(ia);
	lVal *result = lEval(clRoot,form);
	return result;
}

lVal *lispCallFuncIII(const char *symbol, int ia, int ib, int ic){
	lVal *form = lCons(NULL,NULL);
	lRootsValPush(form);
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValInt(ia);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ib);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ic);

	lVal *result = lEval(clRoot,form);
	return result;
}

lVal *lispCallFuncS(const char *symbol, const char *str){
	lVal *form = lCons(NULL,NULL);
	lRootsValPush(form);
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValString(str);
	lVal *result = lEval(clRoot,form);
	return result;
}

lVal *lispCallFuncVII(const char *symbol,const vec va, int ib , int ic){
	lVal *form = lCons(NULL,NULL);
	lRootsValPush(form);
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValVec(va);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ib);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ic);

	lVal *result = lEval(clRoot,form);
	return result;
}

void lispDefineID(const char *prefix, const char *symbol, int val){
	char lName[sizeof(lSymbol)];
	int len = sizeof(lName);

	memset(lName,0,sizeof(lName));
	char *ln = lName;
	const int off = snprintf(lName,len,"%s",prefix);
	if(off > 0){
		ln += off;
		len -= off;
	}
	for(int i=0;i<len;i++){
		u8 c = *symbol++;
		if(isspace((u8)c)){
			*ln++ = '-';
		}else{
			*ln++ = tolower(c);
		}
	}
	lName[sizeof(lName)-1] = 0;
	lispDefineInt(lName,val);
}
