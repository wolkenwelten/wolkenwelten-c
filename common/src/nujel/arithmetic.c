#include "arithmetic.h"
#include "nujel.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lVal *lnfAddV(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecAdd(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfAddF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfAddV(c,t);
		} else if(rv->type == ltInt){
			t->vFloat += (float)rv->vInt;
			continue;
		} else if(rv->type == ltInf)  {
			return rv;
		} else if(rv->type != ltFloat){ continue; }
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
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfAddV(c,t);
		} else if(rv->type == ltFloat){
			t = lnfFloat(c,t);
			t->next = r;
			return lnfAddF(c,t);
		} else if(rv->type == ltInf)  {
			return rv;
		} else if(rv->type != ltInt)  {continue;}
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
	case ltVec:
		return lnfAddV(c,v);
	}
}

static lVal *lnfSubV(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	if(v->next == NULL){
		t->vVec = vecInvert(t->vVec);
		return t;
	}
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecSub(t->vVec,rv->vVec);
	}
	return t;
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
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfSubV(c,t);
		}else if(rv->type == ltInt){
			t->vFloat -= (float)rv->vInt;
			continue;
		}else if(rv->type == ltInf){
			return rv;
		}else if(rv->type != ltFloat){continue;}
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
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfSubV(c,t);
		}else if(rv->type == ltFloat){
			t = lnfFloat(c,t);
			t->next = r;
			return lnfSubF(c,t);
		}else if(rv->type == ltInf)  {
			return rv;
		}else if(rv->type != ltInt)  {continue;}
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
	case ltVec:
		return lnfSubV(c,v);
	}
}

static lVal *lnfMulV(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecMul(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfMulF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfMulV(c,t);
		}else if(rv->type == ltInt){
			t->vFloat *= (float)rv->vInt;
			continue;
		}else if(rv->type == ltInf){
			return rv;
		}else if(rv->type != ltFloat){continue;}
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
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfMulV(c,t);
		}else if(rv->type == ltFloat){
			t = lnfFloat(c,t);
			t->next = r;
			return lnfMulF(c,t);
		}else if(rv->type == ltInf){
			return rv;
		}else if(rv->type != ltInt){continue;}
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
	case ltVec:
		return lnfMulV(c,v);
	}
}

static lVal *lnfDivV(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecDiv(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfDivF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfDivV(c,t);
		if(rv->type == ltInt){
			if(rv->vInt == 0){
				t->type = ltInf;
				return t;
			}
			t->vFloat /= (float)rv->vInt;
			continue;
		}
		}else if(rv->type == ltInf){
			return rv;
		}else if(rv->type != ltFloat){continue;}

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
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfDivV(c,t);
		}else if(rv->type == ltFloat){
			t = lnfFloat(c,t);
			t->next = r;
			return lnfDivF(c,t);
		}else if(rv->type == ltInf){
			return rv;
		}else if(rv->type != ltInt){continue;}

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
	case ltVec:
		return lnfDivV(c,v);
	}
}

static lVal *lnfModV(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecMod(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfModF(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfModV(c,t);
		}else if(rv->type == ltInt){
			if(rv->vInt == 0){
				t->type = ltInf;
				return t;
			}
			t->vFloat = fmodf(t->vFloat,(float)rv->vInt);
			continue;
		} else if(rv->type == ltInf){
			return rv;
		} else if(rv->type != ltFloat){continue;}

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
		if(rv->type == ltVec){
			t = lnfVec(c,t);
			t->next = r;
			return lnfModV(c,t);
		}else if(rv->type == ltFloat){
			t = lnfFloat(c,t);
			t->next = r;
			return lnfModF(c,t);
		}else if(rv->type == ltInf)  {
			return rv;
		}else if(rv->type != ltInt)  {continue;}
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
	case ltVec:
		return lnfModV(c,v);
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
		return lValInt(t->vFloat);
	case ltVec:
		return lValInt(t->vVec.x);
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
	case ltVec:
		return lValFloat(t->vVec.x);
	case ltCString:
		if(t->vCString == NULL){return lValFloat(0);}
		return lValFloat(atof(t->vCString->data));
	case ltString:
		if(t->vString == NULL){return lValFloat(0);}
		return lValFloat(atof(t->vString->data));
	}
}


lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0);
	int i = 0;
	for(lVal *cv = v;cv != NULL;cv = cv->next){
		lVal *t = lEval(c,cv);
		if(t == NULL){break;}
		t = lnfFloat(c,t);
		if(t == NULL){break;}
		for(int ii=i;ii<3;ii++){
			nv.v[ii] = t->vFloat;
		}
		if(++i >= 3){break;}
	}
	return lValVec(nv);
}

lVal *lnfAbs(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValInt(0);}
	if((t->type != ltInt) && (t->type != ltFloat) && (t->type != ltVec)){
		t = lnfFloat(c,t);
	}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(fabsf(t->vFloat));
	case ltInt:
		return lValInt(abs(t->vInt));
	case ltVec:
		return lValVec(vecAbs(t->vVec));
	}
}
