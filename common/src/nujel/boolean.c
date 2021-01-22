#include "boolean.h"
#include "casting.h"

lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lnfBool(c,v);
	return lValBool(a == NULL ? true : !a->vBool);
}

lVal *lnfAnd(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(true);}
	lVal *t = lnfBool(c,v);
	if((t == NULL) || (!t->vBool)){return lValBool(false);}
	return lnfAnd(c,v->vList.cdr);
}

lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lnfBool(c,v);
	if((t != NULL) && t->vBool){return lValBool(true);}
	return lnfOr(c,v->vList.cdr);
}
