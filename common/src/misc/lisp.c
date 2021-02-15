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
#include "../game/itemType.h"
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
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		if(t->vInt > 0){
			msPerTick = t->vInt;
		}
	}
	return lValInt(msPerTick);
}

lVal *wwlnfProf(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	return lValString(profGetReport());
}

lVal *wwlnfProfReset(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	profReset();
	return lValBool(true);
}

lVal *wwlnfNProf(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	return lValString(nprofGetReport());
}

lVal *wwlnfNProfReset(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	nprofReset();
	return lValBool(true);
}

lVal *wwlnfAsmSwitch(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t != NULL){
		t = lnfInt(c,v);
		asmRoutineSupport = t->vInt;
	}
	return lValInt(asmRoutineSupport);
}

void lispDefineInt(const char *symbol, int val){
	lDefineVal(clRoot,symbol,lValInt(val));
}

void lispDefineString(const char *symbol, char *str){
	lDefineVal(clRoot,symbol,lValString(str));
}

lClosure *lispCommonRoot(){
	lClosure *c = lClosureNewRoot();

	lAddNativeFunc(c,"mst!",        "(a)","Sets ms per tick to s",       wwlnfMsPerTick);
	lAddNativeFunc(c,"prof",        "()", "Returns profiler info",       wwlnfProf);
	lAddNativeFunc(c,"prof-reset!", "()", "Resets performance counters", wwlnfProfReset);
	lAddNativeFunc(c,"nprof",       "()", "Return network profiler info",wwlnfNProf);
	lAddNativeFunc(c,"nprof-reset!","()", "Resets network counters",     wwlnfNProfReset);
	lAddNativeFunc(c,"asm-switch!", "(a)","Switches asm/simd routines",  wwlnfAsmSwitch);
	itemTypeLispClosure(c);

	lEval(c,lWrap(lRead((const char *)src_tmp_wwlib_nuj_data)));

	return c;
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
