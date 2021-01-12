#include "arithmetic.h"
#include "nujel.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lVal *lnfAddV(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecAdd(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfAddF(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfAddV(c,lCons(lnfVec(c,t),r));
		} else if(rv->type == ltInt){
			t->vFloat += (float)rv->vInt;
			continue;
		} else if(rv->type == ltInf){
			return rv;
		} else if(rv->type != ltFloat){ continue; }
		t->vFloat += rv->vFloat;
	}
	return t;
}

static lVal *lnfAddI(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfAddV(c,lCons(lnfVec(c,t),r));
		} else if(rv->type == ltFloat){
			return lnfAddF(c,lCons(lnfFloat(c,t),r));
		} else if(rv->type == ltInf)  {
			return rv;
		} else if(rv->type != ltInt)  {continue;}
		t->vInt += rv->vInt;
	}
	return t;
}

lVal *lnfAdd(lClosure *c, lVal *v){
	return lnfAddI(c,v);
}

static lVal *lnfSubV(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	if(v->vList.cdr == NULL){
		t->vVec = vecInvert(t->vVec);
		return t;
	}
	foreach(r,v->vList.cdr){
		if(r == NULL){continue;}
		lVal *rv = lEval(c,r->vList.car);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecSub(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfSubF(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	if(v->vList.cdr == NULL){
		t->vFloat = -t->vFloat;
		return t;
	}
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfSubV(c,lCons(lnfVec(c,t),r));
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
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	if(v->vList.cdr == NULL){
		t->vInt = -t->vInt;
		return t;
	}
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfSubV(c,lCons(lnfVec(c,t),r));
		}else if(rv->type == ltFloat){
			return lnfSubF(c,lCons(lnfFloat(c,t),r));
		}else if(rv->type == ltInf)  {
			return rv;
		}else if(rv->type != ltInt)  {continue;}
		t->vInt -= rv->vInt;
	}
	return t;
}

lVal *lnfSub(lClosure *c, lVal *v){
	return lnfSubI(c,v);
}

static lVal *lnfMulV(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecMul(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfMulF(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfMulV(c,lCons(lnfVec(c,t),r));
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
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfMulV(c,lCons(lnfVec(c,t),r));
		}else if(rv->type == ltFloat){
			return lnfMulF(c,lCons(lnfFloat(c,t),r));
		}else if(rv->type == ltInf){
			return rv;
		}else if(rv->type != ltInt){continue;}
		t->vInt *= rv->vInt;
	}
	return t;
}

lVal *lnfMul(lClosure *c, lVal *v){
	return lnfMulI(c,v);
}

static lVal *lnfDivV(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecDiv(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfDivF(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfDivV(c,lCons(lnfVec(c,t),r));
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
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lValDup(lEval(c,v->vList.car));
	if(t == NULL){return NULL;}
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfDivV(c,lCons(lnfVec(c,t),r));
		}else if(rv->type == ltFloat){
			return lnfDivF(c,lCons(lnfFloat(c,t),r));
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
	return lnfDivI(c,v);
}

static lVal *lnfModV(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltVec){
			rv = lnfVec(c,lValDup(rv));
		}
		t->vVec = vecMod(t->vVec,rv->vVec);
	}
	return t;
}

static lVal *lnfModF(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfModV(c,lCons(lnfVec(c,t),r));
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
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lValDup(t);
	foreach(r,v->vList.cdr){
		lVal *rv = lEval(c,r->vList.car);
		if(rv == NULL){continue;}
		if(rv->type == ltVec){
			return lnfModV(c,lCons(lnfVec(c,t),r));
		}else if(rv->type == ltFloat){
			return lnfModF(c,lCons(lnfFloat(c,t),r));
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
	return lnfModI(c,v);
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
	foreach(cv,v){
		lVal *t = lEval(c,cv->vList.car);
		if(t == NULL){break;}
		if(t->type == ltVec){return t;}
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
	if(v->type == ltList){v = v->vList.car;}
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

lVal *lnfVX(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(t->vVec.x);
	}
}

lVal *lnfVY(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(t->vVec.y);
	}
}

lVal *lnfVZ(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lnfFloat(c,t);
	case ltVec:
		return lValFloat(t->vVec.z);
	}
}
