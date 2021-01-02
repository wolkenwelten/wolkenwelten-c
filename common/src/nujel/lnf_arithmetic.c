#include "lnf_arithmetic.h"
#include "nujel.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lVal *lnfAddF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltInt){
			t->vFloat += (float)rv->vInt;
			continue;
		}
		if(rv->type != ltFloat){continue;}
		t->vFloat += rv->vFloat;
	}
	return t;
}

static lVal *lnfAddI(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltFloat){
			lVal *fr = lnfAddF(c,r);
			fr->vFloat += (float)t->vInt;
			return fr;
		}
		if(rv->type != ltInt)  {continue;}
		t->vInt += rv->vInt;
	}
	return t;
}

lVal *lnfAdd(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	switch(t->type){
	case ltInt:
	default:
		return lnfAddI(c,v);
	case ltFloat:
		return lnfAddF(c,v);
	}
}

static lVal *lnfSubF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	if(v->next == NULL){
		t->vFloat = -t->vFloat;
		return t;
	}
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltInt){
			t->vFloat -= (float)rv->vInt;
			continue;
		}
		if(rv->type != ltFloat){continue;}
		t->vFloat -= rv->vFloat;
	}
	return t;
}

static lVal *lnfSubI(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	if(v->next == NULL){
		t->vInt = -t->vInt;
		return t;
	}
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltFloat){
			lVal *fr = lnfSubF(c,r);
			fr->vFloat -= (float)t->vInt;
			return fr;
		}
		if(rv->type != ltInt)  {continue;}
		t->vInt -= rv->vInt;
	}
	return t;
}

lVal *lnfSub(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	switch(t->type){
	case ltInt:
	default:
		return lnfSubI(c,v);
	case ltFloat:
		return lnfSubF(c,v);
	}
}

static lVal *lnfMulF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltInt){
			t->vFloat *= (float)rv->vInt;
			continue;
		}
		if(rv->type != ltFloat){continue;}
		t->vFloat *= rv->vFloat;
	}
	return t;
}

static lVal *lnfMulI(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltFloat){
			lVal *fr = lnfMulF(c,r);
			fr->vFloat *= (float)t->vInt;
			return fr;
		}
		if(rv->type != ltInt)  {continue;}
		t->vInt *= rv->vInt;
	}
	return t;
}

lVal *lnfMul(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	switch(t->type){
	case ltInt:
	default:
		return lnfMulI(c,v);
	case ltFloat:
		return lnfMulF(c,v);
	}
}

static lVal *lnfDivF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltInt){
			if(rv->vInt == 0){
				t->type = ltInf;
				return t;
			}
			t->vFloat /= (float)rv->vInt;
			continue;
		}
		if(rv->type != ltFloat){continue;}
		if(rv->vFloat == 0.f){
			t->type = ltInf;
			return t;
		}
		t->vFloat /= rv->vFloat;
	}
	return t;
}

static lVal *lnfDivI(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltFloat){
			lVal *fr = lnfDivF(c,r);
			if(fr->vFloat == 0.f){
				t->type = ltInf;
				return t;
			}
			fr->vFloat = (float)t->vInt / fr->vFloat;
			return fr;
		}
		if(rv->type != ltInt){continue;}
		if(rv->vInt == 0){
			t->type = ltInf;
			return t;
		}
		t->vInt /= rv->vInt;
	}
	return t;
}

lVal *lnfDiv(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	switch(t->type){
	case ltInt:
	default:
		return lnfDivI(c,v);
	case ltFloat:
		return lnfDivF(c,v);
	}
}

static lVal *lnfModF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltInt){
			if(rv->vInt == 0){
				t->type = ltInf;
				return t;
			}
			t->vFloat = fmodf(t->vFloat,(float)rv->vInt);
			continue;
		}
		if(rv->type != ltFloat){continue;}
		if(rv->vFloat == 0.f){
			t->type = ltInf;
			return t;
		}
		t->vFloat = fmodf(t->vFloat,rv->vFloat);
	}
	return t;
}

static lVal *lnfModI(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf)  {return rv;}
		if(rv->type == ltFloat){
			lVal *fr = lnfModF(c,r);
			if(fr->vFloat == 0.f){
				t->type = ltInf;
				return t;
			}
			fr->vFloat = fmodf(t->vInt,fr->vFloat);
			return fr;
		}
		if(rv->type != ltInt)  {continue;}
		if(rv->vInt == 0){
			t->type = ltInf;
			return t;
		}
		t->vInt %= rv->vInt;
	}
	return t;
}

lVal *lnfMod(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	switch(t->type){
	case ltInt:
	default:
		return lnfModI(c,v);
	case ltFloat:
		return lnfModF(c,v);
	}
}

lVal *lnfInt(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default: return lValInt(0);
	case ltBool:
		return lValInt(t->vBool ? 1 : 0);
	case ltInt:
		return t;
	case ltFloat:
		return lValInt((int)t->vFloat);
	case ltCString:
		if(t->vCString == NULL){return lValInt(0);}
		return lValInt(atoi(t->vCString->data));
	case ltString:
		if(t->vString == NULL){return lValInt(0);}
		return lValInt(atoi(t->vString->data));
	}
}

lVal *lnfFloat(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default: return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lValFloat(t->vInt);
	case ltCString:
		if(t->vCString == NULL){return lValFloat(0);}
		return lValFloat(atof(t->vCString->data));
	case ltString:
		if(t->vString == NULL){return lValFloat(0);}
		return lValFloat(atof(t->vString->data));
	}
}

lVal *lnfAbs(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValInt(0);}
	if((t->type != ltInt) && (t->type != ltFloat)){
		t = lnfFloat(c,t);
	}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(fabsf(t->vFloat));
	case ltInt:
		return lValInt(abs(t->vInt));
	}
}
