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
	return lValBool(true);;
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
	lAddNativeFunc(c,"mst","Adds a server",wwlnfMsPerTick);
	lAddNativeFunc(c,"prof","Adds a server",wwlnfProf);
	lAddNativeFunc(c,"prof-reset","Adds a server",wwlnfProfReset);
	lAddNativeFunc(c,"nprof","Adds a server",wwlnfNProf);
	lAddNativeFunc(c,"nprof-reset","Adds a server",wwlnfNProfReset);
	lAddNativeFunc(c,"asm-switch","Adds a server",wwlnfAsmSwitch);
	return c;
}
