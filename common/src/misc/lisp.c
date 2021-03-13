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
#include "../nujel/nujel.h"
#include "../nujel/arithmetic.h"
#include "../nujel/casting.h"
#include "../nujel/reader.h"
#include "../misc/profiling.h"
#include "../tmp/assets.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

lClosure *clRoot;

lVal *wwlnfMsPerTick(lClosure *c, lVal *v){
	int newVal = 0;
	getLArgI(newVal);
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
	int newVal = -1;
	getLArgI(newVal);
	if(newVal >= 0){asmRoutineSupport = newVal;}
	return lValInt(asmRoutineSupport);
}

lVal *wwlnfExplode(lClosure *c, lVal *v){
	vec   pos      = vecNOne();
	float strength = 4.f;
	int   style    = 0;

	getLArgV(pos);
	getLArgF(strength);
	getLArgI(style);

	if((pos.x < 0.f) || (pos.y < 0.f) || (pos.z < 0.f) || (strength < .1f)){return NULL;}
	explode(pos,strength,style);
	return NULL;
}

static lVal *wwlnfItemDropNew(lClosure *c, lVal *v){
	vec pos = vecNOne();
	int id  = 0;
	int amt = 0;

	getLArgV(pos);
	getLArgI(id);
	getLArgI(amt);

	if((pos.x < 0.f) || (pos.y < 0.f) || (pos.z < 0.f) || (id <= 0) || (amt <= 0)){return NULL;}
	item itm = itemNew(id,amt);
	itemDropNewP(pos,&itm);
	return NULL;
}

static lVal *wwlnfWVel(lClosure *c, lVal *v){
	if(v != NULL){
		vec nwval = vecZero();
		getLArgV(nwval);
		cloudsSetWind(nwval);
	}
	return lValVec(windVel);
}

static lVal *wwlnfCDen(lClosure *c, lVal *v){
	int cden = -1;
	getLArgI(cden);
	if(cden >= 0){cloudsSetDensity(cden);}
	return lValInt(cloudGDensityMin);
}

static lVal *wwlnfRain(lClosure *c, lVal *v){
	int inten = -1;
	getLArgI(inten);
	if(inten >= 0){weatherSetRainDuration(inten);}
	return lValInt(rainIntensity);
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
	itemTypeLispClosure(c);

	lEval(c,lWrap(lRead((const char *)src_tmp_wwlib_nuj_data)));
	return c;
}

lVal *lispCallFuncI(const char *symbol, int ia){
	lVal *arg  = lCons(lValInt(ia),NULL);
	return lEval(clRoot,lCons(lValSym(symbol),arg));
}

lVal *lispCallFuncVII(const char *symbol,const vec va, int ib , int ic){
	lVal *arg = lCons(lValInt(ic),NULL);
	      arg = lCons(lValInt(ib),arg);
	      arg = lCons(lValVec(va),arg);
	return lEval(clRoot,lCons(lValSym(symbol),arg));
}

void lispDefineID(const char *prefix, const char *symbol, int val){
	char lName[16];
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
