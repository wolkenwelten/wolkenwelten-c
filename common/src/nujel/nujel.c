#include "nujel.h"

#include "arithmetic.h"
#include "boolean.h"
#include "casting.h"
#include "predicates.h"
#include "string.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lVal      lValBuf[1<<18];
uint      lValMax = 0;
lVal     *lValFFree = NULL;

lClosure  lClosureList[1<<10];
uint      lClosureMax   = 0;
lClosure *lClosureFFree = NULL;

lString   lStringList[1<<10];
lString  *lStringFFree = NULL;

lCString   lCStringList[1<<10];
lCString  *lCStringFFree = NULL;

lSymbol symQuote;



void lInit(){
	strncpy(symQuote.c,"quote",15);
	symQuote.c[15] = 0;
	lValMax       = 0;
	lClosureMax   = 0;

	for(uint i=0;i<countof(lValBuf)-1;i++){
		lValBuf[i].type = ltNoAlloc;
		lValBuf[i].vNA  = &lValBuf[i+1];
	}
	lValFFree = &lValBuf[0];

	for(uint i=0;i<countof(lClosureList)-1;i++){
		lClosureList[i].parent = &lClosureList[i+1];
	}
	lClosureFFree = &lClosureList[0];

	for(uint i=0;i<countof(lStringList)-1;i++){
		lStringList[i].next = &lStringList[i+1];
	}
	lStringFFree = &lStringList[0];

	for(uint i=0;i<countof(lCStringList)-1;i++){
		lCStringList[i].next = &lCStringList[i+1];
	}
	lCStringFFree = &lCStringList[0];
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
	return ret;
}
void lClosureFree(lClosure *c){
	if(c == NULL){return;}
	c->parent = lClosureFFree;
	c->data   = NULL;
	lClosureFFree = c;
	lValFree(c->data);
}

lString *lStringAlloc(){
	if(lStringFFree == NULL){
		lPrintError("lString OOM\n");
		return NULL;
	}
	lString *ret = lStringFFree;
	lStringFFree = ret->next;
	return ret;
}
void lStringFree(lString *s){
	if(s == NULL){return;}
	if(s->buf != NULL){free(s->buf);}
	s->buf = s->data = NULL;
	s->next = lStringFFree;
	lStringFFree = s;
}

lCString *lCStringAlloc(){
	if(lCStringFFree == NULL){
		lPrintError("lCString OOM\n");
		return NULL;
	}
	lCString *ret = lCStringFFree;
	lCStringFFree = ret->next;
	return ret;
}

void lCStringFree(lCString *s){
	if(s == NULL){return;}
	s->data = NULL;
	s->next = lCStringFFree;
	lCStringFFree = s;
}

lString *lStringNew(const char *str, unsigned int len){
	lString *s = lStringAlloc();
	s->buf = s->data = malloc(len+1);
	memcpy(s->data,str,len);
	s->data[len] = 0;
	s->len       = len;
	s->bufEnd    = &s->buf[len+1];
	return s;
}

lVal *lValString(const char *c){
	if(c == NULL){return lValNil();}
	lVal *t = lValAlloc();
	t->type = ltString;
	t->vString = lStringNew(c,strlen(c));
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
		dst->vString = lStringNew(src->vString->data,src->vString->len);
	}else if(dst->type == ltCString){
		dst->vString = lStringNew(src->vString->data,(int) (src->vString->bufEnd - src->vString->data));
	}else if(dst->type == ltList){
		dst->vList.car = lValDup(dst->vList.car);
		dst->vList.cdr = lValDup(dst->vList.cdr);
	}
	return dst;
}

lVal *lValNil(){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltNil;
	return ret;
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
	v->type = ltList;
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

void lPrintVal(lVal *v){
	static char buf[256];
	lSPrintVal(v,buf,&buf[sizeof(buf)]);
	printf("%s\n",buf);
}

static lVal *lnfDefine(lClosure *c, lClosure *ec, lVal *v, lVal *(*func)(lClosure *,lSymbol)){
	if((v == NULL) || (v->type != ltList)){return lValNil();}
	lVal *sym = v->vList.car;
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return lValNil();}
	lVal *t = func(c,sym->vSymbol);
	if((t == NULL) || (t->type != ltList)){return lValNil();}
	return t->vList.car = lEval(ec,v->vList.cdr->vList.car);
}
static lVal *lnfDef(lClosure *c, lVal *v){
	return lnfDefine(c,c,v,lDefineClosureSym);
}
static lVal *lnfSet(lClosure *c, lVal *v){
	return lnfDefine(c,c,v,lGetClosureSym);
}

static lVal *lnfCl(lClosure *c, lVal *v){
	if(c == NULL){return lValNil();}
	if(v == NULL){return c->data;}
	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		t = lValDup(t);
		t->vInt--;
		return lnfCl(c->parent,v);
	}
	return c->data;
}

static lVal *lnfClText(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if((t == NULL) || (t->type != ltLambda)){return NULL;}
	return t->vLambda->text;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
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
		t->vList.car = lValNil();
		(void)t;
	}

	return ret;
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	//return lCons(lValSymS(symQuote),lCons(v->vList.car,0));
	return v->vList.car;
}

static lVal *lnfMem(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInt(lMemUsage());
}

static lVal *lnfLet(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lClosure *nc = lClosureNew(c);
	forEach(n,v->vList.car){
		lnfDefine(nc,c,n->vList.car,lDefineClosureSym);
	}
	lVal *ret = NULL;
	//lPrintVal(v->vList.cdr);
	forEach(n,v->vList.cdr){
		ret = lEval(nc,n->vList.car);
	}
	return ret == NULL ? lValNil() : ret;
}

static lVal *lnfRepldo(lClosure *c, lVal *v){
	lVal *ret = NULL;
	forEach(n,v){
		ret = lEval(c,n->vList.car);
	}
	return ret == NULL ? lValNil() : ret;
}

static lVal *lLambda(lClosure *c,lVal *v, lClosure *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	lVal *vn = v;
	lClosure *tmpc = lClosureNew(lambda);
	forEach(n,lambda->data){
		if(vn == NULL){break;}
		lVal *nn = n->vList.car;
		if(nn->type != ltList)             {continue;}
		if(nn->vList.car == NULL)          {continue;}
		if(nn->vList.car->type != ltSymbol){continue;}
		lVal *lv = lDefineClosureSym(tmpc,nn->vList.car->vSymbol);
		lVal *t = lEval(c,vn->vList.car);
		if(t  != NULL && t->type == ltSymbol){ t = lEval(c,t); }
		if(t  != NULL){lValCopy(lv,lCons(t,NULL));}
		if(vn != NULL){vn = vn->vList.cdr;}
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
	if((vsym == NULL) || (vsym->type != ltSymbol)){return lValNil();}
	lVal *t = lDefineClosureSym(c,vsym->vSymbol);
	*t = *nf;
	return t;
}

static lVal *lnfCond(lClosure *c, lVal *v){
	if(v == NULL){return lValNil();}
	forEach(n,v){
		if(n->type != ltList)                 {continue;}
		if(n->vList.car == NULL)              {continue;}
		if(n->vList.car->type != ltList)      {continue;}
		lVal *t = lEval(c,n->vList.car->vList.car);
		if(t == NULL)                         {continue;}
		if(t->type == ltNil)                  {continue;}
		if((t->type == ltBool) && (!t->vBool)){continue;}
		return lLastCar(lApply(c,n->vList.car->vList.cdr,lEval));
	}
	return lValNil();
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL)         {return lValNil();}
	if(v->type != ltList) {return lValNil();}
	lVal *pred = lnfBool(c,v->vList.car);
	v = v->vList.cdr;
	if((pred == NULL) || (pred->vBool == false)){v = v->vList.cdr;}

	return lEval(c,v->vList.car);
}

static lVal *lnfWhen(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *pred = lnfBool(c,v->vList.car);
	if((pred == NULL) || (pred->vBool == false)){return NULL;}
	return lLastCar(lApply(c,v->vList.cdr,lEval));
}

static lVal *lnfCar(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if((t == NULL) || (t->type != ltList)){return NULL;}
	return t->vList.car;
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if((t == NULL) || (t->type != ltList)){return NULL;}
	return t->vList.cdr;
}
static lVal *lnfCons(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltList)){return lCons(NULL,NULL);}
	lVal *car = lEval(c,v->vList.car);
	lVal *cdr = lEval(c,v->vList.cdr != NULL ? v->vList.cdr->vList.car : NULL);
	if((cdr != NULL) && (cdr->type == ltNil)){cdr = NULL;}
	return lCons(car,cdr);
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

	if(strcmp(s.c,"and") == 0)    {return lValNativeFunc(lnfAnd);}
	if(strcmp(s.c,"not") == 0)    {return lValNativeFunc(lnfNot);}
	if(strcmp(s.c,"or") == 0)     {return lValNativeFunc(lnfOr);}

	if(strcmp(s.c,"car") == 0)    {return lValNativeFunc(lnfCar);}
	if(strcmp(s.c,"cdr") == 0)    {return lValNativeFunc(lnfCdr);}
	if(strcmp(s.c,"cons") == 0)   {return lValNativeFunc(lnfCons);}

	if(strcmp(s.c,"mem") == 0)    {return lValNativeFunc(lnfMem);}
	if(strcmp(s.c,"λ") == 0)      {return lValNativeFunc(lnfLambda);}
	if(strcmp(s.c,"lambda") == 0) {return lValNativeFunc(lnfLambda);}
	if(strcmp(s.c,"cl") == 0)     {return lValNativeFunc(lnfCl);}
	if(strcmp(s.c,"cl-text") == 0){return lValNativeFunc(lnfClText);}
	if(strcmp(s.c,"cl-data") == 0){return lValNativeFunc(lnfClData);}

	if(strcmp(s.c,"if") == 0)     {return lValNativeFunc(lnfIf);}
	if(strcmp(s.c,"when") == 0)   {return lValNativeFunc(lnfWhen);}
	if(strcmp(s.c,"cond") == 0)   {return lValNativeFunc(lnfCond);}

	if(strcmp(s.c,"define") == 0) {return lValNativeFunc(lnfDef);}
	if(strcmp(s.c,"let") == 0)    {return lValNativeFunc(lnfLet);}
	if(strcmp(s.c,"repldo") == 0) {return lValNativeFunc(lnfRepldo);}
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

	if(strcmp(s.c,"ansirs") == 0) {return lValNativeFunc(lnfAnsiRS);}
	if(strcmp(s.c,"ansifg") == 0) {return lValNativeFunc(lnfAnsiFG);}
	if(strcmp(s.c,"br") == 0)     {return lValNativeFunc(lnfBr);}
	if(strcmp(s.c,"cat") == 0)    {return lValNativeFunc(lnfCat);}
	if(strcmp(s.c,"len") == 0)    {return lValNativeFunc(lnfLen);}
	if(strcmp(s.c,"substr") == 0) {return lValNativeFunc(lnfSubstr);}

	if(strcmp(s.c,"int?") == 0)   {return lValNativeFunc(lnfIntPred);}
	if(strcmp(s.c,"float?") == 0) {return lValNativeFunc(lnfFloatPred);}
	if(strcmp(s.c,"vec?") == 0)   {return lValNativeFunc(lnfVecPred);}
	if(strcmp(s.c,"nil?") == 0)   {return lValNativeFunc(lnfNilPred);}
	if(strcmp(s.c,"inf?") == 0)   {return lValNativeFunc(lnfInfPred);}
	if(strcmp(s.c,"number?") == 0){return lValNativeFunc(lnfNumberPred);}
	if(strcmp(s.c,"empty?") == 0) {return lValNativeFunc(lnfEmptyPred);}
	if(strcmp(s.c,"neg?") == 0)   {return lValNativeFunc(lnfNegPred);}
	if(strcmp(s.c,"pos?") == 0)   {return lValNativeFunc(lnfPosPred);}
	if(strcmp(s.c,"string?") == 0){return lValNativeFunc(lnfStringPred);}
	if(strcmp(s.c,"zero?") == 0)  {return lValNativeFunc(lnfZero);}

	return lValNil();
}

static lVal *lGetSym(lClosure *c, const lSymbol s){
	if(c == NULL){return NULL;}
	forEach(v,c->data){
		const lVal *n = v->vList.car;
		if(n == NULL)                  {continue;}
		if(n->type != ltList)          {continue;}
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

lVal *lEval(lClosure *c, lVal *v){
	//lPrintVal(v);
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c,v->vSymbol);
	}else if(v->type == ltList){
		lVal *ret = lEval(c,v->vList.car);
		if(ret == NULL){return v;}
		if(ret->type == ltNativeFunc){
			return ret->vNativeFunc(c,v->vList.cdr);
		}else if(ret->type == ltLambda){
			return lLambda(c,v->vList.cdr,ret->vLambda);
		}else if(ret->type == ltList){
			return lEval(c,ret->vList.cdr);
		}
	}
	return v;
}

int lMemUsage(){
	int used=0;
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		used++;
	}
	return used;
}

static void lValGCMark(lVal *v){
	if(v == NULL){return;}
	v->flags |= lfMarked;
	if(v->type == ltList){
		lValGCMark(v->vList.car);
		lValGCMark(v->vList.cdr);
	} else if(v->type == ltLambda) {
		lValGCMark(v->vLambda->data);
		lValGCMark(v->vLambda->text);
	}
}

static void lGCMark(){
	for(uint i=0;i<lValMax;i++){lValBuf[i].flags &= ~lfMarked;}
	//for(;c != NULL;c = c->parent){lValGCMark(c->data);}
	for(uint i=0;i<lClosureMax;i++){
		lValGCMark(lClosureList[i].data);
		lValGCMark(lClosureList[i].text);
	}
}
static void lGCSweep(){
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		if(lValBuf[i].flags & lfMarked) {continue;}
		lValFree(&lValBuf[i]);
	}
}
void lClosureGC(){
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
	return ltNil;
}

lType lTypecastList(lVal *a){
	if((a == NULL) || (a->type != ltList) || (a->vList.car == NULL)){return ltNil;}
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
	}
}
