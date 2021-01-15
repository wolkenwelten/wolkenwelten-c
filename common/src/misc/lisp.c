#include "lisp.h"

#include "../nujel/nujel.h"
#include "../nujel/arithmetic.h"
#include "../nujel/casting.h"
#include "../misc/profiling.h"

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

lVal *lResolveNativeSymCommon(const lSymbol s){
	if(strcmp(s.c,"mst") == 0)            {return lValNativeFunc(wwlnfMsPerTick);}
	if(strcmp(s.c,"prof") == 0)           {return lValNativeFunc(wwlnfProf);}
	if(strcmp(s.c,"prof-reset") == 0)     {return lValNativeFunc(wwlnfProfReset);}
	if(strcmp(s.c,"nprof") == 0)          {return lValNativeFunc(wwlnfNProf);}
	if(strcmp(s.c,"nprof-reset") == 0)    {return lValNativeFunc(wwlnfNProfReset);}

	return lResolveNativeSymBuiltin(s);
}
