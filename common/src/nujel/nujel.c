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
#include "nujel.h"

#include "blockType.h"

#include "../asm/asm.h"
#include "../game/weather/weather.h"
#include "../network/messages.h"
#include "../misc/colors.h"
#include "../misc/profiling.h"
#include "../world/world.h"

#include "../../nujel/lib/api.h"
#include "../../nujel/lib/exception.h"
#include "../../nujel/lib/allocation/roots.h"

extern uint src_tmp_wwlib_nuj_len;
extern u8   src_tmp_wwlib_nuj_data[];

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

static lVal *wwlnfWVelGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValVec(windVel);
}

static lVal *wwlnfWVelSet(lClosure *c, lVal *v){
	(void)c;
	if((v != NULL) && (lCar(v) != NULL) && (lCar(v)->type == ltVec)){
		windSet(castToVec(lCar(v),vecZero()));
	}
	return NULL;
}

static lVal *wwlnfCDenGet(lClosure *c, lVal *v){
	(void)c; (void) v;
	return lValInt(cloudGDensityMin);
}

static lVal *wwlnfCDenSet(lClosure *c, lVal *v){
	(void)c;;
	const int cden = castToInt(lCar(v),-1);
	cloudsSetDensity(cden);
	return lCar(v);
}

static lVal *wwlnfRainGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(rainIntensity);
}

static lVal *wwlnfRainSet(lClosure *c, lVal *v){
	(void)c;
	const int inten = castToInt(lCar(v),-1);
	if(inten >= 0){weatherSetRainIntensity(inten);}
	return lValInt(rainIntensity);
}

static lVal *wwlnfSnowGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(snowIntensity);
}

static lVal *wwlnfSnowSet(lClosure *c, lVal *v){
	(void)c;
	const int inten = castToInt(lCar(v),-1);
	if(inten >= 0){weatherSetSnowIntensity(inten);}
	return lValInt(snowIntensity);
}

static lVal *wwlnfStormGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lList(4,
		RVP(lValSym(":intensity")), RVP(lValInt(stormIntensity)),
		RVP(lValSym(":delta")),     RVP(lValInt(stormDelta)));
}

static lVal *wwlnfStormSet(lClosure *c, lVal *v){
	(void)c;
	stormIntensity = castToInt(lCar(v), stormIntensity);
	stormDelta     = castToInt(lCadr(v),stormDelta);
	if(!isClient){weatherSendUpdate(-1);}
	return NULL;
}

static lVal *wwlnfColorInterpolate(lClosure *c, lVal *v){
	(void)c;
	const int ca  = castToInt(lCar(v),0); v = lCdr(v);
	const int cb  = castToInt(lCar(v),0); v = lCdr(v);
	const float i = castToFloat(lCar(v),0.f);

	return lValInt(colorInterpolate(ca,cb,i));
}

static lVal *wwlnfFluidGet(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());
	if(pos.x < 0){return NULL;}
	return lValInt(worldGetFluid(pos.x,pos.y,pos.z));
}

static lVal *wwlnfFluidSet(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());
	const int level = castToInt(lCadr(v), -1);
	if((pos.x < 0) || (level < 0)){return NULL;}
	worldSetFluid(pos.x, pos.y, pos.z, level);
	return lCar(v);
}

static lVal *wwlnfFireGet(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());
	if(pos.x < 0){return NULL;}
	return lValInt(worldGetFire(pos.x,pos.y,pos.z));
}

static lVal *wwlnfFireSet(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());
	const int level = castToInt(lCadr(v), -1);
	if((pos.x < 0) || (level < 0)){return NULL;}
	worldSetFire(pos.x, pos.y, pos.z, level);
	return lCar(v);
}

static lVal *wwlnfPrint(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return v;}
	lWriteVal(lCar(v));
	return NULL;
}

static lVal *wwlnfError(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return v;}
	lDisplayErrorVal(lCar(v));
	return NULL;
}

void lispDefineInt(const char *symbol, int val){
	lDefineVal(clRoot,symbol,lValInt(val));
}

void lispDefineString(const char *symbol, char *str){
	lDefineVal(clRoot,symbol,lValString(str));
}

static lVal *wwlnfMessageSend(lClosure *c, lVal *v){
	const int to = castToInt(lCar(v), -2);
	if((to < -1) || (to > 32)){
		lExceptionThrowValClo("out-of-bounds", "Out of bounds recipient", lCar(v), c);
	}
	const char *msg = castToString(lCadr(v), NULL);
	if(msg == NULL){
		lExceptionThrowValClo("type-error", "Messages have to be serialized S-Expressions", lCadr(v), c);
	}
	msgNujelMessage(to,msg);
	return NULL;
}

void *lispCommonRootReal(void *a, void *b){
	(void)a; (void)b;
	lInit();
	lClosure *c = lNewRoot();
	void (*specificInit)(lClosure *c) = (void (*)(lClosure *))a;

	lAddNativeFunc(c,"error",           "args",                   "Prints ...args to stderr",                                   wwlnfError);
	lAddNativeFunc(c,"print",           "args",                   "Displays ...args",                                           wwlnfPrint);
	lAddNativeFunc(c,"mst!",            "(a)",                    "Set ms per tick to s",                                       wwlnfMsPerTick);
	lAddNativeFunc(c,"prof",            "()",                     "Return profiler info",                                       wwlnfProf);
	lAddNativeFunc(c,"prof-reset!",     "()",                     "Reset performance counters",                                 wwlnfProfReset);
	lAddNativeFunc(c,"nprof",           "()",                     "Return network profiler info",                               wwlnfNProf);
	lAddNativeFunc(c,"nprof-reset!",    "()",                     "Reset network counters",                                     wwlnfNProfReset);
	lAddNativeFunc(c,"asm-switch!",     "(a)",                    "Switch asm/simd routines",                                   wwlnfAsmSwitch);
	lAddNativeFunc(c,"cloud-threshold", "()",                     "Get the current cloud threshold",                            wwlnfCDenGet);
	lAddNativeFunc(c,"cloud-threshold!","(thresh)",               "Set cloud threshold to &THRESH",                             wwlnfCDenSet);
	lAddNativeFunc(c,"wind-velocity",   "()",                     "Get wind velocity",                                          wwlnfWVelGet);
	lAddNativeFunc(c,"wind-velocity!",  "(&vel)",                 "Set wind velocity to vector &VEL",                           wwlnfWVelSet);
	lAddNativeFunc(c,"rain",            "()",                     "Set rain rate to INTENSITY",                                 wwlnfRainGet);
	lAddNativeFunc(c,"snow",            "()",                     "Set rain rate to INTENSITY",                                 wwlnfSnowGet);
	lAddNativeFunc(c,"rain!",           "(intensity)",            "Set rain rate to INTENSITY",                                 wwlnfRainSet);
	lAddNativeFunc(c,"snow!",           "(intensity)",            "Set rain rate to INTENSITY",                                 wwlnfSnowSet);
	lAddNativeFunc(c,"storm",           "()",                     "Set rain rate to INTENSITY",                                 wwlnfStormGet);
	lAddNativeFunc(c,"storm!",          "(intensity delta)",      "Set rain rate to INTENSITY",                                 wwlnfStormSet);
	lAddNativeFunc(c,"color-inter",     "(a b i)",                "Interpolate between A and B with 0.0 <= i <= 1.0",           wwlnfColorInterpolate);
	lAddNativeFunc(c,"fluid",           "(pos)",                  "Get the fluid level at POS",                                 wwlnfFluidGet);
	lAddNativeFunc(c,"fluid!",          "(pos level)",            "Set the fluid level at POS to LEVEL",                        wwlnfFluidSet);
	lAddNativeFunc(c,"fire",            "(pos)",                  "Get the fluid level at POS",                                 wwlnfFireGet);
	lAddNativeFunc(c,"fire!",           "(pos level)",            "Set the fluid level at POS to LEVEL",                        wwlnfFireSet);
	lAddNativeFunc(c,"message/send*",   "[to msg]",               "Send MSG TO someone",                                        wwlnfMessageSend);
	lOperatorsBlockType(c);

	specificInit(c);

	lLoadS(c,(const char *)src_tmp_wwlib_nuj_data, src_tmp_wwlib_nuj_len);

	return c;
}

lClosure *lispCommonRoot(void (*specificInit)(lClosure *)){
	return lExceptionTryExit(lispCommonRootReal, (void *)specificInit,NULL);
}

void *lispCallFuncReal(void *closure, void *vv){
	lClosure *c = (lClosure *)closure;
	lVal *v = (lVal *)vv;
	return lEval(c,v);
}

lVal *lispCallFunc(const char *symbol, lVal *v){
	const int SP = lRootsGet();
	lVal *form = RVP(lCons(NULL,NULL));
	form->vList.car = lValSym(symbol);
	form->vList.cdr = lCons(v, NULL);

	lVal *ret = lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
	return ret;
}

lVal *lispCallFuncI(const char *symbol, int ia){
	const int SP = lRootsGet();
	lVal *form = RVP(lCons(NULL,NULL));
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValInt(ia);

	lVal *ret = lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
	return ret;
}

lVal *lispCallFuncIII(const char *symbol, int ia, int ib, int ic){
	const int SP = lRootsGet();
	lVal *form = RVP(lCons(NULL,NULL));
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValInt(ia);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ib);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ic);

	lVal *ret = lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
	return ret;
}

lVal *lispCallFuncS(const char *symbol, const char *str){
	const int SP = lRootsGet();
	lVal *form = RVP(lCons(NULL,NULL));
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValString(str);

	lVal *ret = lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
	return ret;
}

lVal *lispCallFuncVII(const char *symbol,const vec va, int ib , int ic){
	const int SP = lRootsGet();
	lVal *form = RVP(lCons(NULL,NULL));
	form->vList.car = lValSym(symbol);
	lVal *l = form->vList.cdr = lCons(NULL,NULL);
	l->vList.car = lValVec(va);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ib);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(ic);

	lVal *ret = lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
	return ret;
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

void nujelReceiveMessage(uint c, const packet *p){
	const int len = strnlen((char *)p->v.u8, 4188);
	lVal *args = lList(2, lValInt(c), lValStringNoCopy((char *)p->v.u8, len));
	lVal *func = lGetClosureSym(clRoot, lSymS("message/receive"));
	lApply(clRoot, args, func, NULL);
}
