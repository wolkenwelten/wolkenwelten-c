#include "lisp.h"

#include "../asm/asm.h"
#include "../nujel/nujel.h"
#include "../nujel/arithmetic.h"
#include "../nujel/casting.h"
#include "../nujel/reader.h"
#include "../misc/profiling.h"
#include "../tmp/assets.h"

#include <string.h>

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

lClosure *lispCommonRoot(){
	lClosure *c = lClosureNewRoot();
	lEval(c,lWrap(lRead((const char *)src_tmp_wwlib_nuj_data)));
	lAddNativeFunc(c,"mst!",        "(a)","Sets ms per tick to s",       wwlnfMsPerTick);
	lAddNativeFunc(c,"prof",        "()", "Returns profiler info",       wwlnfProf);
	lAddNativeFunc(c,"prof-reset!", "()", "Resets performance counters", wwlnfProfReset);
	lAddNativeFunc(c,"nprof",       "()", "Return network profiler info",wwlnfNProf);
	lAddNativeFunc(c,"nprof-reset!","()", "Resets network counters",     wwlnfNProfReset);
	lAddNativeFunc(c,"asm-switch!", "(a)","Switches asm/simd routines",  wwlnfAsmSwitch);
	return c;
}
