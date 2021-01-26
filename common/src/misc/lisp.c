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

lVal *wwlnfNAsmSwitch(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t != NULL){
		t = lnfInt(c,v);
		asmRoutineSupport = t->vInt;
	}
	return lValInt(asmRoutineSupport);
}

lVal *lResolveNativeSymCommon(const lSymbol s){
	if(strcmp(s.c,"mst") == 0)            {return lValNativeFunc(wwlnfMsPerTick);}
	if(strcmp(s.c,"prof") == 0)           {return lValNativeFunc(wwlnfProf);}
	if(strcmp(s.c,"prof-reset") == 0)     {return lValNativeFunc(wwlnfProfReset);}
	if(strcmp(s.c,"nprof") == 0)          {return lValNativeFunc(wwlnfNProf);}
	if(strcmp(s.c,"nprof-reset") == 0)    {return lValNativeFunc(wwlnfNProfReset);}
	if(strcmp(s.c,"asm-switch") == 0)     {return lValNativeFunc(wwlnfNAsmSwitch);}

	return lResolveNativeSymBuiltin(s);
}

lClosure *lispCommonRoot(){
	lClosure *c = lClosureNewRoot();
	lEval(c,lWrap(lRead((const char *)src_tmp_wwlib_nuj_data)));
	return c;
}
