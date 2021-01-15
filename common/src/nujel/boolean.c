#include "boolean.h"
#include "casting.h"

lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lnfBool(c,v);
	a->vBool = !a->vBool;
	return a;
}

lVal *lnfAnd(lClosure *c, lVal *v){
	forEach(n,v){
		lVal *a = lnfBool(c,n->vList.car);
		if(!a->vBool){return a;}
	}
	return v == NULL ? NULL : lValBool(true);
}

lVal *lnfOr(lClosure *c, lVal *v){
	forEach(n,v){
		lVal *a = lnfBool(c,n->vList.car);
		if(a->vBool){return a;}
	}
	return lValBool(false);
}
