#include "nujel.h"

#include "arithmetic.h"
#include "array.h"
#include "boolean.h"
#include "casting.h"
#include "predicates.h"
#include "reader.h"
#include "string.h"

#include "../tmp/assets.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VAL_MAX (1<<16)
#define CLO_MAX (1<<12)
#define STR_MAX (1<<12)
#define ARR_MAX (1<<12)

lVal      lValBuf[VAL_MAX];
uint      lValMax = 0;
lVal     *lValFFree = NULL;

lClosure  lClosureList[CLO_MAX];
uint      lClosureMax   = 0;
lClosure *lClosureFFree = NULL;

lString   lStringList[STR_MAX];
uint      lStringMax   = 0;
lString  *lStringFFree = NULL;

lArray     lArrayList[ARR_MAX];
uint       lArrayMax   = 0;
lArray    *lArrayFFree = NULL;

lSymbol symQuote,symArr;


void lInit(){
	strncpy(symQuote.c,"quote",15);
	symQuote.c[15] = 0;
	strncpy(symArr.c,"arr",15);
	symQuote.c[15] = 0;
	lValMax        = 0;
	lClosureMax    = 0;
	lStringMax     = 0;
	lArrayMax      = 0;

	for(uint i=0;i<VAL_MAX-1;i++){
		lValBuf[i].type = ltNoAlloc;
		lValBuf[i].vNA  = &lValBuf[i+1];
	}
	lValFFree = &lValBuf[0];

	for(uint i=0;i<CLO_MAX-1;i++){
		lClosureList[i].nextFree = &lClosureList[i+1];
	}
	lClosureFFree = &lClosureList[0];

	for(uint i=0;i<STR_MAX-1;i++){
		lStringList[i].nextFree = &lStringList[i+1];
	}
	lStringFFree = &lStringList[0];

	for(uint i=0;i<ARR_MAX-1;i++){
		lArrayList[i].nextFree = &lArrayList[i+1];
	}
	lArrayFFree = &lArrayList[0];
}

lClosure *lClosureAlloc(){
	if(lClosureFFree == NULL){
		lPrintError("lClosure OOM\n");
		return NULL;
	}
	lClosure *ret = lClosureFFree;
	lClosureFFree = ret->nextFree;
	lClosureMax   = MAX(lClosureMax,(uint)(ret-lClosureList) + 1);
	ret->data   = ret->text = NULL;
	ret->parent = NULL;
	ret->nextFree = NULL;
	ret->flags  = 0;
	return ret;
}
void lClosureFree(lClosure *c){
	if(c == NULL){return;}
	c->data = c->text = NULL;
	c->parent   = NULL;
	c->nextFree = lClosureFFree;
	lClosureFFree = c;
}

lArray *lArrayAlloc(){
	if(lArrayFFree == NULL){
		lPrintError("lArray OOM\n");
		return NULL;
	}
	lArray *ret   = lArrayFFree;
	lArrayMax     = MAX(lArrayMax,(uint)(ret-lArrayList) + 1);
	lArrayFFree   = ret->nextFree;
	ret->flags    = 0;
	ret->length   = 0;
	ret->data     = NULL;
	ret->nextFree = NULL;
	return ret;
}

void lArrayFree(lArray *v){
	if(v == NULL){return;}
	free(v->data);
	v->nextFree = lArrayFFree;
	v->data     = NULL;
}

lString *lStringAlloc(){
	if(lStringFFree == NULL){
		lPrintError("lString OOM\n");
		return NULL;
	}
	lString *ret = lStringFFree;

	lStringFFree = ret->nextFree;
	lStringMax   = MAX(lStringMax,(uint)(ret-lStringList) + 1);
	ret->data    = ret->buf = ret->bufEnd = NULL;
	ret->flags   = 0;
	ret->nextFree = NULL;
	return ret;
}
void lStringFree(lString *s){
	if(s == NULL){return;}
	if((s->buf != NULL) && (s->flags & lfHeapAlloc)){
		free((void *)s->buf);
	}
	s->data = s->buf = s->bufEnd = NULL;
	s->nextFree = lStringFFree;
	lStringFFree = s;
}

lString *lStringNew(const char *str, unsigned int len){
	lString *s = lStringAlloc();
	if(s == NULL){return NULL;}
	char *nbuf = malloc(len+1);
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->flags |= lfHeapAlloc;
	s->buf = s->data = nbuf;
	s->bufEnd = &s->buf[len];
	s->nextFree = NULL;
	return s;
}

lVal *lValString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vString = lStringNew(c,strlen(c));
	return t;
}
lVal *lValCString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vString = lStringAlloc();
	t->vString->buf = t->vString->data = c;
	t->vString->bufEnd = t->vString->buf + strlen(c);
	t->vString->flags |= lfConst;
	return t;
}

lVal *lValAlloc(){
	if(lValFFree == NULL){
		lPrintError("lVal OOM\n");
		return NULL;
	}
	lVal *ret  = lValFFree;
	lValFFree  = ret->vNA;
	ret->vNA   = NULL;
	lValMax    = MAX(lValMax,(uint)(ret-lValBuf) + 1);
	return ret;
}
void lValFree(lVal *v){
	if(v == NULL){return;}
	switch(v->type){
	case ltArray:
		lArrayFree(v->vArr);
		v->vArr = NULL;
		break;
	case ltString:
		lStringFree(v->vString);
		break;
	default:
		break;
	}
	v->type   = ltNoAlloc;
	v->vNA    = lValFFree;
	v->flags  = 0;
	lValFFree = v;
}

lVal *lValCopy(lVal *dst, const lVal *src){
	if(dst == NULL){return NULL;}
	if(src == NULL){return NULL;}
	*dst = *src;
	if(dst->type == ltString){
		dst->vString = lStringNew(src->vString->data,lStringLength(src->vString));
	}else if(dst->type == ltPair){
		dst->vList.car = lValDup(dst->vList.car);
		dst->vList.cdr = lValDup(dst->vList.cdr);
	}
	return dst;
}

lVal *lValInf(){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltInf;
	return ret;
}
lVal *lValInt(int v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}
lVal *lValVec(const vec v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltVec;
	ret->vVec = v;
	return ret;
}
lVal *lValFloat(float v){
	lVal *ret   = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type   = ltFloat;
	ret->vFloat = v;
	return ret;
}
lVal *lValBool(bool v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltBool;
	ret->vBool = v;
	return ret;
}

lVal *lValSymS(const lSymbol s){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltSymbol;
	memcpy(&ret->vSymbol,&s,sizeof(s));
	return ret;
}

lVal *lValSym(const char *s){
	lSymbol sym;
	strncpy(sym.c,s,sizeof(sym.c)-1);
	sym.c[sizeof(sym.c)-1] = 0;
	return lValSymS(sym);
}

lVal *lCons(lVal *car, lVal *cdr){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type = ltPair;
	v->vList.car = car;
	v->vList.cdr = cdr;
	return v;
}

lClosure *lClosureNew(lClosure *parent){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = parent;
	return c;
}

void lDisplayVal(lVal *v){
	char buf[8192];
	lSDisplayVal(v,buf,&buf[sizeof(buf)]);
	printf("%s",buf);
}
void lWriteVal(lVal *v){
	char buf[8192];
	lSWriteVal(v,buf,&buf[sizeof(buf)]);
	printf("%s\n",buf);
}

static lVal *lnfDefine(lClosure *c, lClosure *ec, lVal *v, lVal *(*func)(lClosure *,lSymbol)){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = v->vList.car;
	lVal *nv = lEval(ec,v->vList.cdr->vList.car);
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	lVal *t = func(c,sym->vSymbol);
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	if((t->vList.car != NULL) && (t->vList.car->flags & lfConst)){
		return t->vList.car;
	}else{
		return t->vList.car = nv;
	}
}
static lVal *lnfDef(lClosure *c, lVal *v){
	return lnfDefine(c,c,v,lDefineClosureSym);
}
static lVal *lnfSet(lClosure *c, lVal *v){
	return lnfDefine(c,c,v,lGetClosureSym);
}

static lVal *lnfCl(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfCl(c->parent,lCons(lValInt(t->vInt - 1),NULL));
	}
	return c->data != NULL ? c->data : lCons(NULL,NULL);
}

static lVal *lnfClText(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return t->vLambda->text;
	}else if(t->type == ltNativeFunc){
		return lCons(t->vFunc.doc->vList.cdr,NULL);
	}
	return NULL;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t->type == ltLambda){
		return t->vLambda->data;
	}else if(t->type == ltNativeFunc){
		return t->vFunc.doc->vList.car;
	}
	return NULL;
}

static lVal *lnfClLambda(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfClLambda(c->parent,lCons(lValInt(t->vInt - 1),NULL));
	}
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vLambda = c;
	return ret;
}

static lVal *lnfLambda(lClosure *c, lVal *v){
	lClosure *cl = lClosureNew(c);
	if((cl == NULL) || (v == NULL) || (v->vList.car == NULL) || (v->vList.cdr == NULL)){return NULL;}
	cl->text = v->vList.cdr;
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vLambda = cl;

	forEach(n,v->vList.car){
		if(n->vList.car->type != ltSymbol){continue;}
		lVal *t = lDefineClosureSym(cl,n->vList.car->vSymbol);
		t->vList.car = NULL;
		(void)t;
	}

	return ret;
}

static lVal *lnfDynamic(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	ret->vLambda->flags |= lfDynamic;
	return ret;
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	return v->vList.car;
}

static lVal *lnfMem(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	char buf[4096];
	int vals=0,clos=0,arrs=0,strs=0;
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		vals++;
	}
	for(uint i=0;i<lClosureMax;i++){
		if((lClosureList[i].flags & ~0xFF) != 0){continue;}
		clos++;
	}
	for(uint i=0;i<lArrayMax;i++){
		if(lArrayList[i].nextFree  != NULL){continue;}
		arrs++;
	}
	for(uint i=0;i<lStringMax;i++){
		if((lStringList[i].flags & ~0xFF) != 0){continue;}
		strs++;
	}
	snprintf(buf,sizeof(buf)-1,"Vals:%u Closures:%u Arrs:%u Strings:%u",vals,clos,arrs,strs);
	buf[sizeof(buf)-1]=0;
	fprintf(stderr,"%s\n",buf);
	return lValString(buf);
}

static lVal *lnfLet(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lClosure *nc = lClosureNew(c);
	forEach(n,v->vList.car){
		lnfDefine(nc,c,n->vList.car,lDefineClosureSym);
	}
	lVal *ret = NULL;
	forEach(n,v->vList.cdr){
		ret = lEval(nc,n->vList.car);
	}
	return ret == NULL ? NULL : ret;
}

static lVal *lnfBegin(lClosure *c, lVal *v){
	return lLastCar(lApply(c,v,lEval));
}

static inline bool lSymVariadic(lSymbol s){
	const char *p = s.c;
	if((*p == '@') || (*p == '&')){p++;}
	if((*p == '@') || (*p == '&')){p++;}
	if((p[0] == '.') && (p[1] == '.') && (p[2] == '.')){
		return true;
	}
	return false;
}
/*
static inline bool lSymOptional(lSymbol s){
	if(s.c[0] == '&'){return true;}
	if((s.c[0] == '@') && (s.c[1] == '&')){return true;}
	return false;
}*/
static inline bool lSymNoEval(lSymbol s){
	if(s.c[0] == '@'){return true;}
	if((s.c[0] == '&') && (s.c[1] == '@')){return true;}
	return false;
}

static lVal *lLambda(lClosure *c,lVal *v, lClosure *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	lVal *vn = v;
	lClosure *tmpc;
	if(lambda->flags & lfDynamic){
		tmpc = lClosureNew(c);
	}else{
		tmpc = lClosureNew(lambda);
	}
	if(tmpc == NULL){return NULL;}
	tmpc->text = lambda->text;
	forEach(n,lambda->data){
		if(vn == NULL){break;}
		lVal *nn = n->vList.car;
		if(nn->type != ltPair)             {continue;}
		if(nn->vList.car == NULL)          {continue;}
		if(nn->vList.car->type != ltSymbol){continue;}
		const lSymbol csym = nn->vList.car->vSymbol;
		lVal *lv = lDefineClosureSym(tmpc,csym);
		if(lSymVariadic(csym)){
			lVal *t = lSymNoEval(csym) ? vn : lApply(c,vn,lEval);
			lValCopy(lv,lCons(t,NULL));
			break;
		}else{
			lVal *t = lSymNoEval(csym) ? vn->vList.car : lEval(c,vn->vList.car);
			if(t  != NULL && t->type == ltSymbol && !lSymNoEval(csym)){ t = lEval(c,t); }
			if(t  != NULL){lValCopy(lv,lCons(t,NULL));}
			if(vn != NULL){vn = vn->vList.cdr;}
		}
	}

	lVal *ret = NULL;
	forEach(n,lambda->text){ ret = lEval(tmpc,n->vList.car); }
	return ret;
}

lVal *lValNativeFunc(lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type = ltNativeFunc;
	v->vFunc.fp = func;
	v->vFunc.doc = lCons(args,docString);
	v->flags |= lfConst;
	return v;
}

lVal *lnfCond(lClosure *c, lVal *v){
	if(v == NULL)        {return NULL;}
	if(v->type != ltPair){return NULL;}
	lVal *t = v->vList.car;
	lVal *b = lnfBool(c,t->vList.car);
	if((b != NULL) && b->vBool){
		return lLastCar(lApply(c,t->vList.cdr,lEval));
	}
	return lnfCond(c,v->vList.cdr);
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL)         {return NULL;}
	if(v->type != ltPair) {return NULL;}
	lVal *pred = lnfBool(c,v->vList.car);
	v = v->vList.cdr;
	if(v == NULL)         {return NULL;}
	if(((pred == NULL) || (pred->vBool == false)) && (v->vList.cdr != NULL)){v = v->vList.cdr;}
	return lEval(c,v->vList.car);
}

static lVal *lnfCar(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrN(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	return t->vList.car;
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrN(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	return t->vList.cdr;
}
static lVal *lnfCons(lClosure *c, lVal *v){
	lVal *car = lEval(c,lCarOrN(v));
	lVal *cdr = lEval(c,lCadrOrN(v));
	return lCons(car,cdr);
}

static uint getMSecs(){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return (tv.tv_nsec / 1000000) + (tv.tv_sec * 1000);
}

static lVal *lnfMsecs(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInt(getMSecs());
}

lVal *lResolve(lClosure *c, lVal *v){
	v = lEval(c,lCarOrV(v));
	for(int i=0;i<16;i++){
		if((v == NULL) || (v->type != ltSymbol)){break;}
		v = lResolveSym(c,v);
	}
	return v;
}

lVal *lEval(lClosure *c, lVal *v){
	//lWriteVal(v);
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c,v);
	}else if(v->type == ltPair){
		lVal *ret = lEval(c,v->vList.car);
		if(ret == NULL){return v;}
		if(ret->type == ltNativeFunc){
			return ret->vFunc.fp(c,v->vList.cdr);
		}else if(ret->type == ltLambda){
			return lLambda(c,v->vList.cdr,ret->vLambda);
		}else if(ret->type == ltPair){
			return lEval(c,ret);
		}
	}
	return v;
}

lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lEval(c,lCarOrV(v));
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c,func);}
	switch(func->type){
	case ltNativeFunc:
		return func->vFunc.fp(c,lEval(c,lCarOrV(v->vList.cdr)));
	case ltLambda: {
		lVal *t = lCarOrV(v->vList.cdr);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,func->vLambda);}
	default:
		return v;
	}
}

void lDefineVal(lClosure *c, const char *sym, lVal *val){
	lSymbol S;
	strncpy(S.c,sym,sizeof(S.c)-1);
	S.c[sizeof(S.c)-1] = 0;

	lVal *var = lDefineClosureSym(c,S);
	if(var == NULL){return;}
	var->vList.car = val;
}

void lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lSymbol S;
	strncpy(S.c,sym,sizeof(S.c)-1);
	S.c[sizeof(S.c)-1] = 0;

	lVal *var = lDefineClosureSym(c,S);
	if(var == NULL){
		lPrintError("Error adding NFunc %s\n",sym);
		return;
	}
	var->vList.car = lValNativeFunc(func,lRead(args),lValString(doc));
}

static void lAddCoreFuncs(lClosure *c){
	lAddNativeFunc(c,"+",  "(...args)","Addition",      lnfAdd);
	lAddNativeFunc(c,"add","(...args)","Addition",      lnfAdd);
	lAddNativeFunc(c,"-",  "(...args)","Substraction",  lnfSub);
	lAddNativeFunc(c,"sub","(...args)","Substraction",  lnfSub);
	lAddNativeFunc(c,"*",  "(...args)","Multiplication",lnfMul);
	lAddNativeFunc(c,"mul","(...args)","Multiplication",lnfMul);
	lAddNativeFunc(c,"/",  "(...args)","Division",      lnfDiv);
	lAddNativeFunc(c,"div","(...args)","Division",      lnfDiv);
	lAddNativeFunc(c,"%",  "(...args)","Modulo",        lnfMod);
	lAddNativeFunc(c,"mod","(...args)","Modulo",        lnfMod);

	lAddNativeFunc(c,"<",             "(a b)","#t if a < b", lnfLess);
	lAddNativeFunc(c,"less?",         "(a b)","#t if a < b", lnfLess);
	lAddNativeFunc(c,"<=",            "(a b)","#t if a <= b",lnfLessEqual);
	lAddNativeFunc(c,"less-equal?",   "(a b)","#t if a <= b",lnfLessEqual);
	lAddNativeFunc(c,"=",             "(a b)","#t if a == b",lnfEqual);
	lAddNativeFunc(c,"eq?",           "(a b)","#t if a == b",lnfEqual);
	lAddNativeFunc(c,"eqv?",          "(a b)","#t if a == b",lnfEqual);
	lAddNativeFunc(c,"equal?",        "(a b)","#t if a == b",lnfEqual);
	lAddNativeFunc(c,">=",            "(a b)","#t if a >= b",lnfGreaterEqual);
	lAddNativeFunc(c,"greater-equal?","(a b)","#t if a >= b",lnfGreaterEqual);
	lAddNativeFunc(c,">",             "(a b)","#t if a > b", lnfGreater);
	lAddNativeFunc(c,"greater?",      "(a b)","#t if a > b", lnfGreater);
	lAddNativeFunc(c,"zero?",         "(a)",  "#t if a is 0",lnfZero);

	lAddNativeFunc(c,"arr-length","(a)",      "Returns length of array a",                 lnfArrLength);
	lAddNativeFunc(c,"arr-ref",   "(a i)",    "Returns value of array a at position i",    lnfArrRef);
	lAddNativeFunc(c,"arr-set!",  "(a i v)",  "Sets array valus at position i to v",       lnfArrSet);
	lAddNativeFunc(c,"arr-new",   "(l)",      "Allocates a new array of size l",           lnfArrNew);
	lAddNativeFunc(c,"arr",       "(...args)","Creates a new array from its argument list",lnfArr);

	lAddNativeFunc(c,"and","(...args)","#t if all ...args evaluate to true",            lnfAnd);
	lAddNativeFunc(c,"or" ,"(...args)","#t if one member of ...args evaluates to true", lnfOr);
	lAddNativeFunc(c,"not","(a)",      "#t if a is #f, #f if a is #t",                  lnfNot);

	lAddNativeFunc(c,"car", "(l)",  "Returns the car of pair l",                            lnfCar);
	lAddNativeFunc(c,"cdr", "(l)",  "Returns the cdr of pair l",                            lnfCdr);
	lAddNativeFunc(c,"cons","(a b)","Returns a new pair with a as the car and b as the cdr",lnfCons);

	lAddNativeFunc(c,"apply",    "(f l)",         "Evaluates f with list l as arguments",     lnfApply);
	lAddNativeFunc(c,"eval",     "(expr)",        "Evaluates expr",                           lEval);
	lAddNativeFunc(c,"resolve",  "(s)",           "Resolves s until it is no longer a symbol",lResolve);
	lAddNativeFunc(c,"mem",      "()",            "Returns lVals in use",                     lnfMem);
	lAddNativeFunc(c,"λ",        "(args ...body)","Creates a new lambda",                     lnfLambda);
	lAddNativeFunc(c,"lambda",   "(args ...body)","New Lambda",                               lnfLambda);
	lAddNativeFunc(c,"δ",        "(args ...body)","New Dynamic scoped lambda",                lnfDynamic);
	lAddNativeFunc(c,"dynamic",  "(args ...body)","New Dynamic scoped lambda",                lnfDynamic);
	lAddNativeFunc(c,"cl",       "(i)",           "Returns closure",                          lnfCl);
	lAddNativeFunc(c,"cl-lambda","(i)",           "Returns closure as a lambda",              lnfClLambda);
	lAddNativeFunc(c,"cl-text",  "(f)",           "Returns closures text segment",            lnfClText);
	lAddNativeFunc(c,"cl-data",  "(f)",           "Returns closures data segment",            lnfClData);

	lAddNativeFunc(c,"if",  "(pred? then ...else)","Evalutes then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddNativeFunc(c,"cond","(...c)",              "Contains at least 1 cond block of form (pred? ...body) and evaluates and returns the first where pred? is #t",lnfCond);

	lAddNativeFunc(c,"define","(s v)",         "Define a new symbol s and link it to value v",                      lnfDef);
	lAddNativeFunc(c,"let",   "(args ...body)","Creates a new closure with args bound in which to evaluate ...body",lnfLet);
	lAddNativeFunc(c,"begin", "(...body)",     "Evaluates ...body in order and returns the last result",            lnfBegin);
	lAddNativeFunc(c,"quote", "(v)",           "Returns v as is without evaluating",                                lnfQuote);
	lAddNativeFunc(c,"set!",  "(s v)",         "Binds a new value v to already defined symbol s",                   lnfSet);

	lAddNativeFunc(c,"abs","(a)","Returns the absolute value of a",     lnfAbs);
	lAddNativeFunc(c,"pow","(a b)","Returns a raised to the power of b",lnfPow);
	lAddNativeFunc(c,"sqrt","(a)","Returns the squareroot of a",        lnfSqrt);
	lAddNativeFunc(c,"floor","(a)","Rounds a down",                     lnfFloor);
	lAddNativeFunc(c,"ceil","(a)","Rounds a up",                        lnfCeil);
	lAddNativeFunc(c,"round","(a)","Rounds a",                          lnfRound);

	lAddNativeFunc(c,"vx","(v)","Returns x part of vector v",lnfVX);
	lAddNativeFunc(c,"vy","(v)","Returns x part of vector v",lnfVY);
	lAddNativeFunc(c,"vz","(v)","Returns x part of vector v",lnfVZ);

	lAddNativeFunc(c,"bool", "(a)","Casts a to bool",  lnfBool);
	lAddNativeFunc(c,"int",  "(a)","Casts a to int",   lnfInt);
	lAddNativeFunc(c,"float","(a)","Casts a to float", lnfFloat);
	lAddNativeFunc(c,"vec",  "(a)","Casts a to vec",   lnfVec);
	lAddNativeFunc(c,"str",  "(a)","Casts a to string",lnfCat);

	lAddNativeFunc(c,"msecs","()","Returns monotonic msecs",lnfMsecs);

	lAddNativeFunc(c,"ansi-reset","()",  "Ansi reset code",                 lnfAnsiRS);
	lAddNativeFunc(c,"ansi-fg",   "(a)", "Returns Ansi fg color code for a",lnfAnsiFG);
	lAddNativeFunc(c,"br",        "(&a)","Returns &a=1 linebreaks",         lnfBr);

	lAddNativeFunc(c,"cat",           "(...args)",       "ConCATenates ...args into a single string",                                               lnfCat);
	lAddNativeFunc(c,"str-len",       "(s)",             "Returns length of string s",                                                              lnfStrlen);
	lAddNativeFunc(c,"str-up",        "(s)",             "Returns a copy of string s all uppercased",                                               lnfStrUp);
	lAddNativeFunc(c,"str-down",      "(s)",             "Returns a copy of string s all lowercased",                                               lnfStrDown);
	lAddNativeFunc(c,"str-capitalize","(s)",             "Returns a copy of string s capitalized",                                                  lnfStrCap);
	lAddNativeFunc(c,"substr",        "(s &start &stop)","Returns a copy of string s starting at position &start=0 and ending at &stop=(str-len s)",lnfSubstr);
	lAddNativeFunc(c,"str->sym",      "(s)",             "Converts string s to a symbol",                                                           lnfStrSym);
	lAddNativeFunc(c,"sym->str",      "(s)",             "Converts symbol s to a string",                                                           lnfSymStr);

	lAddNativeFunc(c,"int?",   "(a)","#t if a is of type int",   lnfIntPred);
	lAddNativeFunc(c,"float?", "(a)","#t if a is of type float", lnfFloatPred);
	lAddNativeFunc(c,"vec?",   "(a)","#t if a is of type vec",   lnfVecPred);
	lAddNativeFunc(c,"bool?",  "(a)","#t if a is of type bool",  lnfBoolPred);
	lAddNativeFunc(c,"nil?",   "(a)","#t if a is of type nil",   lnfNilPred);
	lAddNativeFunc(c,"inf?",   "(a)","#t if a is of type inf",   lnfInfPred);
	lAddNativeFunc(c,"pair?",  "(a)","#t if a is of type pair",  lnfPairPred);
	lAddNativeFunc(c,"string?","(a)","#t if a is of type string",lnfStringPred);

	lDefineVal(c,"π",  lConst(lValFloat(PI)));
	lDefineVal(c,"PI", lConst(lValFloat(PI)));
}

lClosure *lClosureNewRoot(){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = NULL;
	c->flags |= lfNoGC;
	lAddCoreFuncs(c);
	lEval(c,lWrap(lRead((const char *)src_tmp_stdlib_nuj_data)));
	return c;
}

static lVal *lGetSym(lClosure *c, const lSymbol s){
	if(c == NULL){return NULL;}
	forEach(v,c->data){
		const lVal *n = v->vList.car;
		if(n == NULL)                  {continue;}
		if(n->type != ltPair)          {continue;}
		const lVal *sym = n->vList.car;
		if(sym->type != ltSymbol)      {continue;}
		if(sym->vSymbol.v[0] != s.v[0]){continue;}
		if(sym->vSymbol.v[1] != s.v[1]){continue;}
		return n->vList.cdr;
	}
	return NULL;
}

lVal *lGetClosureSym(lClosure *c, const lSymbol s){
	if(c == NULL){return NULL;}
	lVal *t = lGetSym(c,s);
	return t != NULL ? t : lGetClosureSym(c->parent,s);
}

lVal *lDefineClosureSym(lClosure *c,const lSymbol s){
	if(c == NULL){return NULL;}
	lVal *get = lGetSym(c,s);
	if(get != NULL){return get;}
	lVal *t = lCons(lValSymS(s),lCons(NULL,NULL));
	if(t == NULL){return NULL;}
	if(c->data == NULL){
		c->data = lCons(t,NULL);
	}else{
		lVal *cdr = NULL;
		for(cdr = c->data;(cdr != NULL) && (cdr->vList.cdr != NULL);cdr = cdr->vList.cdr){}
		if(cdr == NULL){return NULL;}
		cdr->vList.cdr = lCons(t,NULL);
	}
	return t->vList.cdr;
}

lVal *lResolveSym(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltSymbol)){return NULL;}
	lVal *ret = lGetClosureSym(c,v->vSymbol);
	return ret == NULL ? v : ret->vList.car;
}

lVal *lApply(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if((c == NULL) || (v == NULL)){return NULL;}
	//lPrintVal(v);
	lVal *ret = NULL, *cc = NULL;

	forEach(t,v){
		lVal *ct = func(c,t->vList.car);
		if(ct == NULL){continue;}
		ct = lCons(ct,NULL);
		if(ret == NULL){ret = ct;}
		if(cc  != NULL){cc->vList.cdr = ct;}
		cc = ct;
	}

	return ret;
}

static void lClosureGCMark(lClosure *c);
static void lValGCMark(lVal *v);
static void lArrayGCMark(lArray *v);

static void lValGCMark(lVal *v){
	if((v == NULL) || (v->flags & lfMarked)){return;} // Circular refs
	v->flags |= lfMarked;
	if(v->type == ltPair){
		lValGCMark(v->vList.car);
		lValGCMark(v->vList.cdr);
	} else if(v->type == ltLambda) {
		lClosureGCMark(v->vLambda);
	} else if(v->type == ltArray) {
		lArrayGCMark(v->vArr);
	} else if(v->type == ltString) {
		v->vString->flags |= lfMarked;
	}
}

static void lClosureGCMark(lClosure *c){
	if(c == NULL){return;}
	c->flags |= lfMarked;
	lValGCMark(c->data);
	lValGCMark(c->text);
	lClosureGCMark(c->parent);
}

static void lArrayGCMark(lArray *v){
	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
}

static void lGCUnmark(){
	for(uint i=0;i<lValMax    ;i++){lValBuf[i].flags      &= ~lfMarked;}
	for(uint i=0;i<lClosureMax;i++){lClosureList[i].flags &= ~lfMarked;}
	for(uint i=0;i<lStringMax;i++) {lStringList[i].flags  &= ~lfMarked;}
	for(uint i=0;i<lArrayMax;i++)  {lArrayList[i].flags   &= ~lfMarked;}
}

static void lGCMark(){
	for(uint i=0;i<lValMax    ;i++){
		if(!(lValBuf[i].flags & lfNoGC)){continue;}
		lValGCMark(&lValBuf[i]);
	}
	for(uint i=0;i<lClosureMax;i++){
		if(!(lClosureList[i].flags & lfNoGC)){continue;}
		lClosureGCMark(&lClosureList[i]);
	}
	for(uint i=0;i<lStringMax;i++){
		if(!(lStringList[i].flags & lfNoGC)){continue;}
		lStringList[i].flags |= lfMarked;
	}
	for(uint i=0;i<lArrayMax;i++){
		if(!(lArrayList[i].flags & lfNoGC)){continue;}
		lArrayGCMark(&lArrayList[i]);
	}
}

static void lGCSweep(){
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		if(lValBuf[i].flags & lfMarked) {continue;}
		lValFree(&lValBuf[i]);
	}
	for(uint i=0;i<lStringMax;i++){
		if(!(lStringList[i].nextFree != NULL)){continue;}
		lStringFree(&lStringList[i]);
	}
	for(uint i=0;i<lArrayMax;i++){
		if(!(lArrayList[i].nextFree != NULL)){continue;}
		lArrayFree(&lArrayList[i]);
	}
	for(uint i=0;i<lClosureMax;i++){
		if(lClosureList[i].nextFree != NULL){continue;}
		lClosureFree(&lClosureList[i]);
	}
	printf("\n");
}

static void lClosureDoGC(){
	//fprintf(stderr,"Pre-GC MEM:\n");
	//lnfMem(NULL,NULL);
	lGCUnmark();
	lGCMark();
	lGCSweep();
	//fprintf(stderr,"Post-GC MEM:\n");
	//lnfMem(NULL,NULL);
}

void lClosureGC(){
	static uint calls = 0;
	if(++calls > (1<<16)){
		calls = 0;
		lClosureDoGC();
		return;
	}
	if((lValMax < (VAL_MAX/2)) && (lClosureMax < (CLO_MAX/2)) && (lArrayMax < (ARR_MAX / 2)) && (lStringMax < (STR_MAX /2))){return;}
	if((lValMax > (VAL_MAX - 4096)) || (lClosureMax > (CLO_MAX - 256)) || (lArrayMax > (ARR_MAX - 128)) || (lStringMax > (STR_MAX - 256))){
		calls = 0;
		lClosureDoGC();
		return;
	}
	if(calls < 4096){return;}
	calls = 0;
	lClosureDoGC();
}

lType lTypecast(const lType a,const lType b){
	if((a == ltInf)   || (b == ltInf))  {return ltInf;}
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt)  ){return ltInt;}
	if((a == ltBool)  || (b == ltBool) ){return ltBool;}
	if(a == b){ return a;}
	return ltNoAlloc;
}

lType lTypecastList(lVal *a){
	if((a == NULL) || (a->type != ltPair) || (a->vList.car == NULL)){return ltNoAlloc;}
	lType ret = a->vList.car->type;
	forEach(t,a->vList.cdr){ret = lTypecast(ret,t->vList.car->type);}
	return ret;
}

lVal *lCast(lClosure *c, lVal *v, lType t){
	switch(t){
	default:
		return v;
	case ltString:
		return lApply(c,v,lnfString);
	case ltInt:
		return lApply(c,v,lnfInt);
	case ltFloat:
		return lApply(c,v,lnfFloat);
	case ltVec:
		return lApply(c,v,lnfVec);
	case ltInf:
		return lApply(c,v,lnfInf);
	case ltBool:
		return lApply(c,v,lnfBool);
	case ltNoAlloc:
		return NULL;
	}
}
