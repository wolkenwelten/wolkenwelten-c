#include "boolean.h"
#include "casting.h"

lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lnfBool(c,v);
	a->vBool = !a->vBool;
	return a;
}

lVal *lnfAnd(lClosure *c, lVal *v){
	lVal *t = lCast(c,v,ltBool);
	forEach(n,t){if(!n->vList.car->vBool){return n->vList.car;}}
	return v == NULL ? NULL : lValBool(true);
}

lVal *lnfOr(lClosure *c, lVal *v){
	lVal *t = lCast(c,v,ltBool);
	forEach(n,t){if(n->vList.car->vBool){return n->vList.car;}}
	return v == NULL ? NULL : lValBool(false);
}
