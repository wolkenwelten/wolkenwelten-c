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

#define VAL_MAX (1<<18)
#define CLO_MAX (1<<14)
#define STR_MAX (1<<14)
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
	lValMax       = 0;
	lClosureMax   = 0;
	lStringMax    = 0;
	lArrayMax     = 0;

	for(uint i=0;i<VAL_MAX-1;i++){
		lValBuf[i].type = ltNoAlloc;
		lValBuf[i].vNA  = &lValBuf[i+1];
	}
	lValFFree = &lValBuf[0];

	for(uint i=0;i<CLO_MAX-1;i++){
		lClosureList[i].parent = &lClosureList[i+1];
	}
	lClosureFFree = &lClosureList[0];

	for(uint i=0;i<STR_MAX-1;i++){
		lStringList[i].next = &lStringList[i+1];
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
	lClosureFFree = ret->parent;
	lClosureMax   = MAX(lClosureMax,(uint)(ret-lClosureList) + 1);
	ret->data = NULL;
	ret->text = NULL;
	ret->flags = 0;
	return ret;
}
void lClosureFree(lClosure *c){
	if(c == NULL){return;}
	c->parent = lClosureFFree;
	c->data   = NULL;
	c->flags  = 0;
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
	lStringFFree = ret->next;
	lStringMax   = MAX(lStringMax,(uint)(ret-lStringList) + 1);
	ret->data    = ret->buf = ret->bufEnd = NULL;
	ret->flags   = 0;
	return ret;
}
void lStringFree(lString *s){
	if(s == NULL){return;}
	if((s->buf != NULL) && (s->flags & lfHeapAlloc)){
		free((void *)s->buf);
	}
	s->buf = s->data = NULL;
	s->next = lStringFFree;
	lStringFFree = s;
}

lString *lStringNew(const char *str, unsigned int len){
	lString *s = lStringAlloc();
	char *nbuf = malloc(len+1);
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->flags |= lfHeapAlloc;
	s->buf = s->data = nbuf;
 	s->bufEnd = &s->buf[len];
	return s;
}

lVal *lValString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	t->type = ltString;
	t->vString = lStringNew(c,strlen(c));
	return t;
}
lVal *lValCString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
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

lClosure *lClosureNewRoot(){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = NULL;
	c->flags |= lfNoGC;
	lEval(c,lWrap(lRead((const char *)src_tmp_stdlib_nuj_data)));
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
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	lVal *t = func(c,sym->vSymbol);
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	return t->vList.car = lEval(ec,v->vList.cdr->vList.car);
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
	if((t == NULL) || (t->type != ltLambda)){return NULL;}
	return t->vLambda->text;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if((t == NULL) || (t->type != ltLambda)){return NULL;}
	return t->vLambda->data;
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
	return lValInt(lMemUsage());
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
	if((s.c[0] == '.') && (s.c[1] == '.') && (s.c[2] == '.')){
		return true;
	}else if((s.c[0] == '&') && (s.c[1] == '.') && (s.c[2] == '.') && (s.c[3] == '.')){
		return true;
	}
	return false;
}

static inline bool lSymNoEval(lSymbol s){
	return s.c[0] == '&';
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

lVal *lValNativeFunc(lVal *(*func)(lClosure *,lVal *)){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type = ltNativeFunc;
	v->vNativeFunc = func;
	return v;
}

lVal *lClosureAddNF(lClosure *c, const char *sym, lVal *(*func)(lClosure *,lVal *)){
	lVal *nf = lValNativeFunc(func);
	lVal *vsym = lValSym(sym);
	if((vsym == NULL) || (vsym->type != ltSymbol)){return NULL;}
	lVal *t = lDefineClosureSym(c,vsym->vSymbol);
	*t = *nf;
	return t;
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
		v = lResolveSym(c,v->vSymbol);
	}
	return v;
}

lVal *lEval(lClosure *c, lVal *v){
	//lWriteVal(v);
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c,v->vSymbol);
	}else if(v->type == ltPair){
		lVal *ret = lEval(c,v->vList.car);
		if(ret == NULL){return v;}
		if(ret->type == ltNativeFunc){
			return ret->vNativeFunc(c,v->vList.cdr);
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
	if(func->type == ltSymbol){func = lResolveSym(c,func->vSymbol);}
	switch(func->type){
	case ltNativeFunc:
		return func->vNativeFunc(c,lEval(c,lCarOrV(v->vList.cdr)));
	case ltLambda: {
		lVal *t = lCarOrV(v->vList.cdr);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,func->vLambda);}
	default:
		return v;
	}
}

lVal *lResolveNativeSymBuiltin(const lSymbol s){
	if(s.c[1] == 0){
		switch(s.c[0]){
		case '+': return lValNativeFunc(lnfAdd);
		case '-': return lValNativeFunc(lnfSub);
		case '*': return lValNativeFunc(lnfMul);
		case '/': return lValNativeFunc(lnfDiv);
		case '%': return lValNativeFunc(lnfMod);
		case '<': return lValNativeFunc(lnfLess);
		case '=': return lValNativeFunc(lnfEqual);
		case '>': return lValNativeFunc(lnfGreater);
		}
	}
	if((s.c[2] == 0) && (s.c[1] == '=')){
		switch(s.c[0]){
		case '<': return lValNativeFunc(lnfLessEqual);
		case '>': return lValNativeFunc(lnfGreaterEqual);
		}
	}

	if(strcmp(s.c,"arr-length") == 0){return lValNativeFunc(lnfArrLength);}
	if(strcmp(s.c,"arr-ref") == 0)   {return lValNativeFunc(lnfArrRef);}
	if(strcmp(s.c,"arr-set!") == 0)  {return lValNativeFunc(lnfArrSet);}
	if(strcmp(s.c,"arr-new") == 0)   {return lValNativeFunc(lnfArrNew);}
	if(strcmp(s.c,"arr") == 0)       {return lValNativeFunc(lnfArr);}

	if(strcmp(s.c,"and") == 0)    {return lValNativeFunc(lnfAnd);}
	if(strcmp(s.c,"not") == 0)    {return lValNativeFunc(lnfNot);}
	if(strcmp(s.c,"or") == 0)     {return lValNativeFunc(lnfOr);}

	if(strcmp(s.c,"car") == 0)    {return lValNativeFunc(lnfCar);}
	if(strcmp(s.c,"cdr") == 0)    {return lValNativeFunc(lnfCdr);}
	if(strcmp(s.c,"cons") == 0)   {return lValNativeFunc(lnfCons);}

	if(strcmp(s.c,"apply") == 0)  {return lValNativeFunc(lnfApply);}
	if(strcmp(s.c,"eval") == 0)   {return lValNativeFunc(lEval);}
	if(strcmp(s.c,"resolve") == 0){return lValNativeFunc(lResolve);}
	if(strcmp(s.c,"mem") == 0)    {return lValNativeFunc(lnfMem);}
	if(strcmp(s.c,"λ") == 0)      {return lValNativeFunc(lnfLambda);}
	if(strcmp(s.c,"lambda") == 0) {return lValNativeFunc(lnfLambda);}
	if(strcmp(s.c,"δ") == 0)      {return lValNativeFunc(lnfDynamic);}
	if(strcmp(s.c,"dynamic") == 0){return lValNativeFunc(lnfDynamic);}
	if(strcmp(s.c,"cl") == 0)     {return lValNativeFunc(lnfCl);}
	if(strcmp(s.c,"cl-text") == 0){return lValNativeFunc(lnfClText);}
	if(strcmp(s.c,"cl-data") == 0){return lValNativeFunc(lnfClData);}

	if(strcmp(s.c,"if") == 0)     {return lValNativeFunc(lnfIf);}
	if(strcmp(s.c,"cond") == 0)   {return lValNativeFunc(lnfCond);}

	if(strcmp(s.c,"define") == 0) {return lValNativeFunc(lnfDef);}
	if(strcmp(s.c,"let") == 0)    {return lValNativeFunc(lnfLet);}
	if(strcmp(s.c,"begin") == 0)  {return lValNativeFunc(lnfBegin);}
	if(strcmp(s.c,"quote") == 0)  {return lValNativeFunc(lnfQuote);}
	if(strcmp(s.c,"set!") == 0)   {return lValNativeFunc(lnfSet);}

	if(strcmp(s.c,"add") == 0)    {return lValNativeFunc(lnfAdd);}
	if(strcmp(s.c,"div") == 0)    {return lValNativeFunc(lnfDiv);}
	if(strcmp(s.c,"sub") == 0)    {return lValNativeFunc(lnfSub);}
	if(strcmp(s.c,"mul") == 0)    {return lValNativeFunc(lnfMul);}
	if(strcmp(s.c,"mod") == 0)    {return lValNativeFunc(lnfMod);}
	if(strcmp(s.c,"modulo") == 0) {return lValNativeFunc(lnfMod);}
	if(strcmp(s.c,"abs") == 0)    {return lValNativeFunc(lnfAbs);}
	if(strcmp(s.c,"vx") == 0)     {return lValNativeFunc(lnfVX);}
	if(strcmp(s.c,"vy") == 0)     {return lValNativeFunc(lnfVY);}
	if(strcmp(s.c,"vz") == 0)     {return lValNativeFunc(lnfVZ);}

	if(strcmp(s.c,"int") == 0)    {return lValNativeFunc(lnfInt);}
	if(strcmp(s.c,"float") == 0)  {return lValNativeFunc(lnfFloat);}
	if(strcmp(s.c,"vec") == 0)    {return lValNativeFunc(lnfVec);}

	if(strcmp(s.c,"msecs") == 0)     {return lValNativeFunc(lnfMsecs);}
	if(strcmp(s.c,"ansi-reset") == 0){return lValNativeFunc(lnfAnsiRS);}
	if(strcmp(s.c,"ansi-fg") == 0)   {return lValNativeFunc(lnfAnsiFG);}
	if(strcmp(s.c,"br") == 0)        {return lValNativeFunc(lnfBr);}
	if(strcmp(s.c,"cat") == 0)       {return lValNativeFunc(lnfCat);}
	if(strcmp(s.c,"str-len") == 0)   {return lValNativeFunc(lnfStrlen);}
	if(strcmp(s.c,"substr") == 0)    {return lValNativeFunc(lnfSubstr);}
	if(strcmp(s.c,"str->sym") == 0)  {return lValNativeFunc(lnfStrSym);}
	if(strcmp(s.c,"sym->str") == 0)  {return lValNativeFunc(lnfSymStr);}

	if(strcmp(s.c,"int?") == 0)    {return lValNativeFunc(lnfIntPred);}
	if(strcmp(s.c,"float?") == 0)  {return lValNativeFunc(lnfFloatPred);}
	if(strcmp(s.c,"vec?") == 0)    {return lValNativeFunc(lnfVecPred);}
	if(strcmp(s.c,"bool?") == 0)   {return lValNativeFunc(lnfBoolPred);}
	if(strcmp(s.c,"boolean?") == 0){return lValNativeFunc(lnfBoolPred);}
	if(strcmp(s.c,"nil?") == 0)    {return lValNativeFunc(lnfNilPred);}
	if(strcmp(s.c,"null?") == 0)   {return lValNativeFunc(lnfNilPred);}
	if(strcmp(s.c,"inf?") == 0)    {return lValNativeFunc(lnfInfPred);}
	if(strcmp(s.c,"eq?") == 0)     {return lValNativeFunc(lnfEqual);}
	if(strcmp(s.c,"pair?") == 0)   {return lValNativeFunc(lnfPairPred);}
	if(strcmp(s.c,"string?") == 0) {return lValNativeFunc(lnfStringPred);}
	if(strcmp(s.c,"zero?") == 0)   {return lValNativeFunc(lnfZero);}

	return NULL;
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
	if((c == NULL) || (lGetSym(c,s) != NULL)){return NULL;}
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

lVal *lResolveClosureSym(lClosure *c, const lSymbol s){
	lVal *v = lGetClosureSym(c,s);
	return v == NULL ? NULL : v->vList.car;
}

lVal *lResolveSym(lClosure *c, const lSymbol s){
	lVal *v = lResolveClosureSym(c,s);
	return v != NULL ? v : lResolveNativeSym(s);
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

int lMemUsage(){
	int used=0;
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		used++;
	}
	return used;
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
	}
}

static void lClosureGCMark(lClosure *c){
	lValGCMark(c->data);
	lValGCMark(c->text);
}

static void lArrayGCMark(lArray *v){
	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
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

static void lGCUnmark(){
	for(uint i=0;i<lValMax    ;i++){lValBuf[i].flags      &= ~lfMarked;}
	for(uint i=0;i<lClosureMax;i++){lClosureList[i].flags &= ~lfMarked;}
	for(uint i=0;i<lStringMax;i++) {lStringList[i].flags  &= ~lfMarked;}
	for(uint i=0;i<lArrayMax;i++)  {lArrayList[i].flags   &= ~lfMarked;}
}

static void lGCSweep(){
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		if(lValBuf[i].flags & lfMarked) {continue;}
		lValFree(&lValBuf[i]);
	}
	for(uint i=0;i<lClosureMax;i++){
		if(lClosureList[i].flags & lfMarked){continue;}
		lClosureFree(&lClosureList[i]);
	}
	for(uint i=0;i<lStringMax;i++){
		if(!(lStringList[i].flags & lfNoGC)){continue;}
		lStringFree(&lStringList[i]);
	}
	for(uint i=0;i<lArrayMax;i++){
		if(!(lArrayList[i].flags & lfNoGC)){continue;}
		lArrayFree(&lArrayList[i]);
	}
}
void lClosureGC(){
	if((lValMax < (VAL_MAX/2)) && (lClosureMax < (CLO_MAX/2))){return;}
	lGCUnmark();
	lGCMark();
	lGCSweep();
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
