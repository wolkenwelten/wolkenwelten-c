#include "boolean.h"

lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lEval(c,v);
	if(a == NULL)        {return lValBool(true);}
	if(a->type == ltNil) {return lValBool(true);}
	if(a->type != ltBool){return lValBool(true);}
	if(!a->vBool)        {return lValBool(true);}
	return lValBool(false);
}

lVal *lnfAnd(lClosure *c, lVal *v){
	foreach(n,v){
		lVal *a = lEval(c,n->vList.car);
		if(a == NULL)        {return lValBool(false);}
		if(a->type == ltNil) {return a;}
		if(a->type != ltBool){continue;}
		if(a->vBool == false){return a;}
	}
	return lValBool(true);
}

lVal *lnfOr(lClosure *c, lVal *v){
	foreach(n,v){
		lVal *a = lEval(c,n->vList.car);
		if(a == NULL)        {continue;}
		if(a->type == ltNil) {continue;}
		if(a->type != ltBool){return a;}
		if(a->vBool == true) {return a;}
	}
	return lValBool(false);
}
