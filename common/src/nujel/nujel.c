/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "nujel.h"

#include "arithmetic.h"
#include "array.h"
#include "binary.h"
#include "boolean.h"
#include "casting.h"
#include "predicates.h"
#include "reader.h"
#include "string.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lGCRuns = 0;

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 1;
uint     lValFFree  = 0;

lClosure lClosureList[CLO_MAX];
uint     lClosureActive = 0;
uint     lClosureMax    = 1;
uint     lClosureFFree  = 0;

lString  lStringList[STR_MAX];
uint     lStringActive = 0;
uint     lStringMax    = 1;
uint     lStringFFree  = 0;

lArray   lArrayList[ARR_MAX];
uint     lArrayActive = 0;
uint     lArrayMax    = 1;
uint     lArrayFFree  = 0;

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncActive = 0;
uint     lNFuncMax    = 1;
uint     lNFuncFFree  = 0;

lVec     lVecList[VEC_MAX];
uint     lVecActive = 0;
uint     lVecMax    = 1;
uint     lVecFFree  = 0;

lSymbol lSymbolList[VEC_MAX];
uint    lSymbolActive = 0;
uint    lSymbolMax    = 1;
uint    lSymbolFFree  = 0;

char dispWriteBuf[1<<16];
lSymbol *symNull,*symQuote,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symBegin,*symStringAt,*symIntAt,*symFloatAt,*symVecAt;


u64 randomValueSeed;
static inline u64 getRandom(){
	randomValueSeed = ((randomValueSeed * 1103515245)) + 12345;
	return ((randomValueSeed&0xFFFF)<<16) | ((randomValueSeed>>16)&0xFFFF);
}

static uint getMSecs(){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return (tv.tv_nsec / 1000000) + (tv.tv_sec * 1000);
}

void lInit(){
	randomValueSeed = getMSecs();

	lValActive      = 0;
	lValMax         = 1;

	lClosureActive  = 0;
	lClosureMax     = 1;

	lStringActive   = 0;
	lStringMax      = 1;

	lArrayActive    = 0;
	lArrayMax       = 1;

	lNFuncActive    = 0;
	lNFuncMax       = 1;

	lVecActive      = 0;
	lVecMax         = 1;

	lSymbolActive   = 0;
	lSymbolMax      = 1;

	symNull     = lSymS("");
	symQuote    = lSymS("quote");
	symArr      = lSymS("arr");
	symIf       = lSymS("if");
	symCond     = lSymS("cond");
	symWhen     = lSymS("when");
	symUnless   = lSymS("unless");
	symLet      = lSymS("let");
	symBegin    = lSymS("begin");
	symStringAt = lSymS("string@");
	symIntAt    = lSymS("int@");
	symFloatAt  = lSymS("float@");
	symVecAt    = lSymS("vec@");
}

static void lVecFree(uint i){
	if((i == 0) || (i >= lVecMax)){return;}
	lVec *v = &lVecList[i];
	if(v->nextFree != 0){return;}
	lVecActive--;
	v->nextFree   = lVecFFree;
	v->flags      = 0;
	lVecFFree = i;
}

static uint lVecAlloc(){
	lVec *ret;
	if(lVecFFree == 0){
		if(lVecMax >= VEC_MAX-1){
			lPrintError("lVec OOM ");
			return 0;
		}
		ret = &lVecList[lVecMax++];
	}else{
		ret = &lVecList[lVecFFree & VEC_MASK];
		lVecFFree = ret->nextFree;
	}
	lVecActive++;
	*ret = (lVec){0};
	return ret - lVecList;
}

static void lNFuncFree(uint i){
	if((i == 0) || (i >= lNFuncMax)){return;}
	lNFunc *nfn = &lNFuncList[i];
	if(nfn->nextFree != 0){return;}
	lNFuncActive--;
	nfn->fp       = NULL;
	nfn->doc      = NULL;
	nfn->nextFree = lNFuncFFree;
	nfn->flags    = 0;
	lNFuncFFree   = i;
}

static uint lNFuncAlloc(){
	lNFunc *ret;
	if(lNFuncFFree == 0){
		if(lNFuncMax >= NFN_MAX-1){
			lPrintError("lNFunc OOM ");
			return 0;
		}
		ret = &lNFuncList[lNFuncMax++];
	}else{
		ret = &lNFuncList[lNFuncFFree & NFN_MASK];
		lNFuncFFree = ret->nextFree;
	}
	lNFuncActive++;
	*ret = (lNFunc){0};
	return ret - lNFuncList;
}

uint lClosureAlloc(){
	lClosure *ret;
	if(lClosureFFree == 0){
		if(lClosureMax >= CLO_MAX-1){
			lPrintError("lClosure OOM ");
			return 0;
		}
		ret = &lClosureList[lClosureMax++];
	}else{
		ret = &lClosureList[lClosureFFree & CLO_MASK];
		lClosureFFree = ret->nextFree;
	}
	lClosureActive++;
	*ret = (lClosure){0};
	ret->flags = lfUsed;
	return ret - lClosureList;
}

void lClosureFree(uint c){
	if((c == 0) || (c >= lClosureMax)){return;}
	lClosure *clo = &lClosureList[c];
	if(!(clo->flags & lfUsed)){return;}
	lClosureActive--;
	clo->nextFree   = lClosureFFree;
	clo->flags      = 0;
	lClosureFFree = c;
}

u32 lArrayAlloc(){
	lArray *ret;
	if(lArrayFFree == 0){
		if(lArrayMax >= ARR_MAX-1){
			lPrintError("lArray OOM ");
			return 0;
		}
		ret = &lArrayList[lArrayMax++];
	}else{
		ret = &lArrayList[lArrayFFree & ARR_MASK];;
		lArrayFFree = ret->nextFree;
	}
	lArrayActive++;
	*ret = (lArray){0};
	return ret - lArrayList;
}

void lArrayFree(u32 v){
	v = v & ARR_MASK;
	if((v == 0) || (v >= lArrayMax)){return;}
	lArrayActive--;
	free(lArrayList[v].data);
	lArrayList[v].data = NULL;
	lArrayList[v].nextFree = lArrayFFree;
	lArrayFFree = v;
}

u32 lStringAlloc(){
	lString *ret;
	if(lStringFFree == 0){
		if(lStringMax >= STR_MAX){
			lPrintError("lString OOM ");
			return 0;
		}
		ret = &lStringList[lStringMax++];
	}else{
		ret = &lStringList[lStringFFree & STR_MASK];
		lStringFFree  = ret->nextFree;
	}
	lStringActive++;
	*ret = (lString){0};
	return ret - lStringList;
}

void lStringFree(u32 v){
	v = v & STR_MASK;
	if((v == 0) || (v > lStringMax)){return;}
	lString *s = &lStringList[v & STR_MASK];
	if((s->buf != NULL) && (s->flags & lfHeapAlloc)){
		free((void *)s->buf);
		s->buf = NULL;
	}
	lStringActive--;
	s->nextFree = lStringFFree;
	lStringFFree = v;
}

u32 lStringNew(const char *str, unsigned int len){
	const u32 i = lStringAlloc();
	if(i == 0){return 0;}
	lString *s = &lStringList[i & STR_MASK];
	if(s == NULL){return 0;}
	char *nbuf = malloc(len+1);
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->flags |= lfHeapAlloc;
	s->buf    = s->data = nbuf;
	s->bufEnd = &s->buf[len];
	return i;
}

u32 lStringDup(uint oi){
	lString *os = &lStringList[oi & STR_MASK];
	uint len = os->bufEnd - os->buf;
	const char *str = os->data;
	const u32 i = lStringAlloc();
	if(i == 0){return 0;}
	lString *s = &lStringList[i & STR_MASK];
	if(s == NULL){return 0;}
	char *nbuf = malloc(len+1);
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->flags |= lfHeapAlloc;
	s->buf    = s->data = nbuf;
	s->bufEnd = &s->buf[len];
	return i;
}


lVal *lValString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vCdr = lStringNew(c,strlen(c));
	if(t->vCdr == 0){
		lValFree(t);
		return NULL;
	}
	return t;
}
lVal *lValCString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vCdr = lStringAlloc();
	if(t->vCdr == 0){
		lValFree(t);
		return NULL;
	}
	lStrBuf(t)    =  lStrData(t) = c;
	lStrEnd(t)    = &lStrBuf(t)[strlen(c)];
	lStrFlags(t) |=  lfConst;
	return t;
}

lVal *lValAlloc(){
	lVal *ret;
	if(lValFFree == 0){
		if(lValMax >= VAL_MAX-1){
			lPrintError("lVal OOM ");
			return NULL;
		}
		ret = &lValList[lValMax++];
	}else{
		ret       = &lValList[lValFFree & VAL_MASK];
		lValFFree = ret->vCdr;
	}
	lValActive++;
	*ret = (lVal){0};
	return ret;
}

void lGUIWidgetFree(lVal *v);
void lValFree(lVal *v){
	if((v == NULL) || (v->type == ltNoAlloc)){return;}
	if(v->type == ltLambda){
		lClo(v->vCdr).refCount--;
	}else if(v->type == ltGUIWidget){
		lGUIWidgetFree(v);
	}
	lValActive--;
	v->type   = ltNoAlloc;
	v->vCdr   = lValFFree;
	lValFFree = v - lValList;
}

lVal *lValCopy(lVal *dst, const lVal *src){
	if((dst == NULL) || (src == NULL)){return NULL;}
	*dst = *src;
	if(dst->type == ltString){
		dst->vCdr = lStringNew(lStrData(src),lStringLength(&lStr(src)));
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
	ret->vCdr = lVecAlloc();
	if(ret->vCdr == 0){
		lValFree(ret);
		return NULL;
	}
	lVecV(ret->vCdr) = v;
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

lSymbol *lSymSL(const char *str, uint len){
	char buf[32];
	len = MIN(sizeof(buf)-1,len);
	memcpy(buf,str,len);
	buf[len] = 0;
	return lSymS(buf);
}

lSymbol *lSymS(const char *str){
	for(uint i = 0;i<lSymbolMax;i++){
		if(strcmp(str,lSymbolList[i].c)){continue;}
		return &lSymbolList[i];
	}
	if(lSymbolMax >= SYM_MAX){
		fprintf(stderr,"lSym Overflow\n");
		return NULL;
	}
	snprintf(lSymbolList[lSymbolMax].c,sizeof(lSymbolList[0].c),"%s",str);
	return &lSymbolList[lSymbolMax++];
}

lVal *lValSymS(const lSymbol *s){
	if(s == NULL){return NULL;}
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltSymbol;
	ret->vCdr = lvSymI(s);
	return ret;
}

lVal *lValSym(const char *s){
	return lValSymS(lSymS(s));
}

lVal *lCons(lVal *car, lVal *cdr){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type = ltPair;
	v->vList.car = car;
	v->vList.cdr = cdr;
	return v;
}

uint lClosureNew(uint parent){
	const uint i = lClosureAlloc();
	if(i == 0){return 0;}
	lClosure *c = &lClosureList[i];
	c->parent = parent;
	lClosure *p = &lClosureList[parent & CLO_MASK];
	p->refCount++;
	return i;
}

/* TODO: Both seem to write outside of buf if v gets too long */
void lDisplayVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	printf("%s",dispWriteBuf);
}

void lDisplayErrorVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	fprintf(stderr,"%s",dispWriteBuf);
}

void lWriteVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,false);
	printf("%s\n",dispWriteBuf);
}

lVal *lSearchClosureSym(uint c, lVal *ret, const char *str, uint len){
	if(c == 0){return ret;}

	forEach(n,lCloData(c)){
		lVal *e = lCaar(n);
		if((e == NULL) || (e->type != ltSymbol)){continue;}
		lSymbol *sym = lvSym(e->vCdr);
		if(sym == NULL){continue;}
		if(strncmp(sym->c,str,len)){continue;}
		ret = lCons(e,ret);
	}
	return lSearchClosureSym(lCloParent(c),ret,str,len);
}

static lVal *lnfDefine(uint c, lClosure *ec, lVal *v, lVal *(*func)(uint ,lSymbol *)){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = lCar(v);
	lVal *nv = NULL;
	if(lCdr(v) != NULL){
		nv = lEval(ec,lCadr(v));
	}
	if(sym->type != ltSymbol){sym = lEval(&lClo(c),sym);}
	if(sym->type != ltSymbol){return NULL;}
	lSymbol *lsym = lvSym(sym->vCdr);
	if((lsym != NULL) && (lsym->c[0] == ':')){return NULL;}
	lVal *t = func(c,lsym);
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	if((lCar(t) != NULL) && (lCar(t)->flags & lfConst)){
		return lCar(t);
	}else{
		t->vList.car = nv;
		return lCar(t);
	}
}

static lVal *lUndefineClosureSym(uint c, lVal *s){
	if(c == 0){return lValBool(false);}

	lVal *lastPair = lCloData(c);
	forEach(v,lCloData(c)){
		lVal *n = lCar(v);
		if((n == NULL) || (n->type != ltPair))  {break;}
		const lVal *sym = lCar(n);
		if(lSymCmp(s,sym) == 0){
			lastPair->vList.cdr = lCdr(v);
			return lValBool(true);
		}
		lastPair = v;
	}
	return lUndefineClosureSym(lCloParent(c),s);
}
static lVal *lnfUndef(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = lCar(v);
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	return lUndefineClosureSym(c - lClosureList,sym);
}
static lVal *lnfDef(lClosure *c, lVal *v){
	return lnfDefine(c - lClosureList,c,v,lDefineClosureSym);
}
static lVal *lnfSet(lClosure *c, lVal *v){
	return lnfDefine(c - lClosureList,c,v,lGetClosureSym);
}

static lVal *lSymTable(lClosure *c, lVal *v, int off, int len){
	(void)v;
	if((c == NULL) || (len == 0)){return v;}
	forEach(n,c->data){
		lVal *entry = lCadar(n);
		if(entry == NULL){continue;}
		if((entry->type != ltNativeFunc) && (entry->type != ltLambda)){continue;}

		if(off > 0){--off; continue;}
		v = lCons(lCaar(n),v);
		if(--len <= 0){return v;}
	}
	if(c->parent == 0){return v;}
	return lSymTable(&lClo(c->parent),v,off,len);
}

static lVal *lnfSymTable(lClosure *c, lVal *v){
	lVal *loff = lnfInt(c,lEval(c,lCar(v)));
	lVal *llen = lnfInt(c,lEval(c,lCadr(v)));
	int off = loff->vInt;
	int len = llen->vInt;
	if(len <= 0){len = 1<<16;}
	return lSymTable(c,NULL,off,len);
}

static int lSymCount(lClosure *c, int ret){
	if(c == NULL){return ret;}
	forEach(n,c->data){
		lVal *entry = lCadar(n);
		if(entry == NULL){continue;}
		if((entry->type != ltNativeFunc) && (entry->type != ltLambda)){continue;}
		++ret;
	}
	if(c->parent == 0){return ret;}
	return lSymCount(&lClo(c->parent),ret);
}

static lVal *lnfSymCount(lClosure *c, lVal *v){
	(void)v;
	return lValInt(lSymCount(c,0));
}

lVal *lnfBegin(lClosure *c, lVal *v){
	lVal *ret = NULL;
	forEach(n,v){
		ret = lEval(c,lCar(n));
	}
	return ret;
}

static lVal *lnfCl(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,lCar(v)));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfCl(&lClo(c->parent),lCons(lValInt(t->vInt - 1),NULL));
	}
	return c->data != NULL ? c->data : lCons(NULL,NULL);
}

static lVal *lnfClText(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,lCar(v));
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCloText(t->vCdr);
	}else if(t->type == ltNativeFunc){
		return lCons(lCdr(lNFN(t->vCdr).doc),NULL);
	}
	return NULL;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,lCar(v));
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCloData(t->vCdr);
	}else if(t->type == ltNativeFunc){
		return lCar(lNFN(t->vCdr).doc);
	}
	return NULL;
}

static lVal *lnfClLambda(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,lCar(v)));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfClLambda(&lClo(c->parent),lCons(lValInt(t->vInt - 1),NULL));
	}
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vCdr = c - lClosureList;
	c->refCount++;
	return ret;
}

static lVal *lnfLambda(lClosure *c, lVal *v){
	const uint cli = lClosureNew(c - lClosureList);
	if(cli == 0){return NULL;}
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){return NULL;}
	lCloText(cli) = lCdr(v);
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vCdr = cli;

	forEach(n,lCar(v)){
		if(lGetType(lCar(n)) != ltSymbol){continue;}
		lVal *t = lDefineClosureSym(cli,lGetSymbol(lCar(n)));
		t->vList.car = NULL;
		(void)t;
	}

	return ret;
}

static lVal *lnfDynamic(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	if(ret == NULL){return NULL;}
	lClo(ret->vCdr).flags |= lfDynamic;
	return ret;
}

static lVal *lnfObject(lClosure *c, lVal *v){
	const uint cli = lClosureNew(c - lClosureList);
	if(cli == 0){return NULL;}
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vCdr = cli;
	lClo(cli).flags |= lfObject;
	lnfBegin(&lClo(cli),v);

	return ret;
}

static lVal *lnfSelf(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(c->flags & lfObject){
		lVal *t = lValAlloc();
		t->type = ltLambda;
		t->vCdr = c - lClosureList;
		return t;
	}
	if(c->parent == 0){return NULL;}
	return lnfSelf(&lClosureList[c->parent],v);
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	return lCar(v);
}

static lVal *lnfMemInfo(lClosure *c, lVal *v){
	(void)c; (void)v;
	lVal *ret = NULL;
	ret = lCons(lValInt(lSymbolMax),ret);
	ret = lCons(lValSym(":symbol"),ret);
	ret = lCons(lValInt(lNFuncActive),ret);
	ret = lCons(lValSym(":native-function"),ret);
	ret = lCons(lValInt(lStringActive),ret);
	ret = lCons(lValSym(":string"),ret);
	ret = lCons(lValInt(lClosureActive),ret);
	ret = lCons(lValSym(":array"),ret);
	ret = lCons(lValInt(lArrayActive),ret);
	ret = lCons(lValSym(":vector"),ret);
	ret = lCons(lValInt(lVecActive),ret);
	ret = lCons(lValSym(":closure"),ret);
	ret = lCons(lValInt(lValActive),ret);
	ret = lCons(lValSym(":value"),ret);
	return ret;
}

static lVal *lnfLet(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	const uint nci = lClosureNew(c - lClosureList);
	if(nci == 0){return NULL;}
	lClosure *nc = &lClosureList[nci & CLO_MASK];
	forEach(n,lCar(v)){
		lnfDefine(nci,c,lCar(n),lDefineClosureSym);
	}
	lVal *ret = NULL;
	forEach(n,lCdr(v)){
		ret = lEval(nc,lCar(n));
	}
	c->refCount--;
	return ret == NULL ? NULL : ret;
}

static inline bool lSymVariadic(lSymbol *s){
	const char *p = s->c;
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
static inline bool lSymNoEval(lSymbol *s){
	if(s->c[0] == '@'){return true;}
	if((s->c[0] == '&') && (s->c[1] == '@')){return true;}
	return false;
}

static lVal *lLambda(lClosure *c,lVal *v, lClosure *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	if(lambda->flags & lfObject){
		return lnfBegin(lambda,v);
	}
	lVal *vn = v;
	uint tmpci = 0;
	if(lambda->flags & lfDynamic){
		tmpci = lClosureNew(lCloI(c));
	}else{
		tmpci = lClosureNew(lCloI(lambda));
	}
	if(tmpci == 0){return NULL;}
	lClosure *tmpc = &lClo(tmpci);
	tmpc->text = lambda->text;
	forEach(n,lambda->data){
		if(vn == NULL){break;}
		lVal *nn = lCar(n);
		if(lGetType(lCar(nn)) != ltSymbol){continue;}
		lSymbol *csym = lGetSymbol(lCar(nn));
		lVal *lv = lDefineClosureSym(tmpci,csym);
		if(lSymVariadic(csym)){
			lVal *t = lSymNoEval(csym) ? vn : lApply(c,vn,lEval);
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			break;
		}else{
			lVal *t = lSymNoEval(csym) ? lCar(vn) : lEval(c,lCar(vn));
			if(t  != NULL && t->type == ltSymbol && !lSymNoEval(csym)){t = lEval(c,t);}
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			if(vn != NULL){vn = lCdr(vn);}
		}
	}

	lVal *ret = NULL;
	forEach(n,lambda->text){
		ret = lEval(tmpc,lCar(n));
	}
	if(tmpc->refCount == 0){
		lClosureFree(tmpci);
	}
	return ret;
}

lVal *lValNativeFunc(lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type    = ltNativeFunc;
	v->flags  |= lfConst;
	v->vCdr    = lNFuncAlloc();
	if(v->vCdr == 0){
		lValFree(v);
		return NULL;
	}
	lNFunc *fn = &lNFN(v->vCdr);
	fn->fp     = func;
	fn->doc    = lCons(args,docString);
	return v;
}

lVal *lnfCond(lClosure *c, lVal *v){
	if(v == NULL)        {return NULL;}
	if(v->type != ltPair){return NULL;}
	lVal *t = lCar(v);
	lVal *b = lnfBool(c,lCar(t));
	if((b != NULL) && b->vBool){
		return lLastCar(lApply(c,lCdr(t),lEval));
	}
	return lnfCond(c,lCdr(v));
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL)         {return NULL;}
	if(v->type != ltPair) {return NULL;}
	lVal *pred = lnfBool(c,lCar(v));
	v = lCdr(v);
	if(v == NULL)         {return NULL;}
	if(((pred == NULL) || (pred->vBool == false)) && (lCdr(v) != NULL)){v = lCdr(v);}
	return lEval(c,lCar(v));
}

static lVal *lnfCar(lClosure *c, lVal *v){
	return lCar(lEval(c,lCar(v)));
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	return lCdr(lEval(c,lCar(v)));
}

static lVal *lnfCons(lClosure *c, lVal *v){
	return lCons(lEval(c,lCar(v)),lEval(c,lCadr(v)));
}
static lVal *lnfSetCar(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *car = NULL;
	if((v != NULL) && (v->type == ltPair) && (lCdr(v) != NULL)){car = lEval(c,lCadr(v));}
	t->vList.car = car;
	return t;
}
static lVal *lnfSetCdr(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *cdr = NULL;
	if((v != NULL) && (v->type == ltPair) && (lCdr(v) != NULL)){cdr = lEval(c,lCadr(v));}
	t->vList.cdr = cdr;
	return t;
}

static lVal *lnfLastPair(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	for(lVal *r=t;r != NULL;r = lCdr(r)){
		if(lCdr(r) == NULL){return r;}
	}
	return NULL;
}

static lVal *lnfRandom(lClosure *c, lVal *v){
	int n = 0;
	v = getLArgI(c,v,&n);
	if(n == 0){
		return lValInt(getRandom());
	}else{
		return lValInt(getRandom() % n);
	}
}

static lVal *lnfRandomSeed(lClosure *c, lVal *v){
	if(v != NULL){
		int n = 0;
		v = getLArgI(c,v,&n);
		randomValueSeed = n;
	}
	return lValInt(randomValueSeed);
}

static lVal *lnfMsecs(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(getMSecs());
}

lVal *lResolve(lClosure *c, lVal *v){
	v = lEval(c,lCar(v));
	for(int i=0;i<16;i++){
		if((v == NULL) || (v->type != ltSymbol)){break;}
		v = lResolveSym(c - lClosureList,v);
	}
	return v;
}

lVal *lEval(lClosure *c, lVal *v){
	//lWriteVal(v);
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c - lClosureList,v);
	}else if(v->type == ltPair){
		lVal *ret = lEval(c,lCar(v));
		if(ret == NULL){return v;}
		switch(ret->type){
		default:
			return v;
		case ltNativeFunc:
			return lNFN(ret->vCdr).fp(c,lCdr(v));
		case ltLambda:
			return lLambda(c,lCdr(v),&lClo(ret->vCdr));
		case ltPair:
			return lEval(c,ret);
		case ltString:
			return v->vList.cdr == NULL ? ret : lEval(c,lCons(lValSymS(symStringAt),v));
		case ltInt:
			return v->vList.cdr == NULL ? ret : lEval(c,lCons(lValSymS(symIntAt),v));
		case ltFloat:
			return v->vList.cdr == NULL ? ret : lEval(c,lCons(lValSymS(symFloatAt),v));
		case ltVec:
			return v->vList.cdr == NULL ? ret : lEval(c,lCons(lValSymS(symVecAt),v));
		case ltArray:
			return v->vList.cdr == NULL ? ret : lnfArrRef(c,v);
		}
	}
	return v;
}

lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lEval(c,lCar(v));
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c - lClosureList,func);}
	switch(func->type){
	case ltNativeFunc:
		if(lNFN(func->vCdr).fp == NULL){return v;}
		return lNFN(func->vCdr).fp(c,lEval(c,lCadr(v)));
	case ltLambda: {
		lVal *t = lCadr(v);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,&lClo(func->vCdr));}
	default:
		return v;
	}
}

void lDefineVal(lClosure *c, const char *str, lVal *val){
	lVal *var = lDefineClosureSym(lCloI(c),lSymS(str));
	if(var == NULL){return;}
	var->vList.car = val;
}

lVal *lnfRead(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	uint dup = lStringDup(t->vCdr);
	if(dup == 0){return NULL;}
	t = lReadString(&lStringList[dup]);
	if((t != NULL) && (t->type == ltPair) && (lCar(t) != NULL) && (lCdr(t) == NULL)){
		return lCar(t);
	}else{
		return t;
	}
}

void lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lVal *lNF   = lValNativeFunc(func,lRead(args),lValString(doc)); // Generate the Value first so each symbol points to the same value
	const char *cur = sym;

	// Run at most 64 times, just a precaution
	for(int i=0;i<64;i++){
		uint len;
		for(len=0;len < sizeof(lSymbol);len++){ // Find the end of the current token, either space or 0
			if(cur[len] == 0)    {break;}
			if(isspace((u8)cur[len])){break;}
		}
		lVal *var = lDefineClosureSym(lCloI(c),lSymSL(cur,len));
		if(var == NULL){
			lPrintError("Error adding NFunc %s\n",sym);
			return;
		}
		var->vList.car = lNF;
		for(;len<32;len++){ // Advance to the next non whitespace character
			if(cur[len] == 0)     {return;} // Or return if we reached the final 0 byte
			if(!isspace((u8)cur[len])){break;}
		}
		cur += len;
	}
	lPrintError("Quite the amount of aliases we have there (%s)\n",sym);
}

static lVal *lnfTime(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(time(NULL));
}

static lVal *lnfStrftime(lClosure *c, lVal *v){
	int timestamp = 0;
	const char *format = "%Y-%m-%d %H:%M:%S";

	v = getLArgI(c,v,&timestamp);
	v = getLArgS(c,v,&format);

	char buf[1024];
	time_t ts = timestamp;
	struct tm *info = localtime(&ts);
	strftime(buf,sizeof(buf),format,info);

	return lValString(buf);
}

static lVal *lnfTypeOf(lClosure *c, lVal *v){
	v = lEval(c,lCar(v));
	if(v == NULL){return lValSym(":nil");}
	switch(v->type){
	case ltNoAlloc:    return lValSym(":no-alloc");
	case ltBool:       return lValSym(":bool");
	case ltPair:       return lValSym(":pair");
	case ltLambda:     return lValSym(":lambda");
	case ltInt:        return lValSym(":int");
	case ltFloat:      return lValSym(":float");
	case ltVec:        return lValSym(":vec");
	case ltString:     return lValSym(":string");
	case ltSymbol:     return lValSym(":symbol");
	case ltNativeFunc: return lValSym(":native-function");
	case ltInf:        return lValSym(":infinity");
	case ltArray:      return lValSym(":array");
	case ltGUIWidget:  return lValSym(":gui-widget");
	}
	return lValSym(":nil");
}

static void lAddPlatformVars(lClosure *c){
	#if defined(__HAIKU__)
	lDefineVal(c, "OS", lConst(lValString("Haiku")));
	#elif defined(__APPLE__)
	lDefineVal(c, "OS", lConst(lValString("Macos")));
	#elif defined(__EMSCRIPTEN__)
	lDefineVal(c, "OS", lConst(lValString("Emscripten")));
	#elif defined(__MINGW32__)
	lDefineVal(c, "OS", lConst(lValString("Windows")));
	#elif defined(__linux__)
	lDefineVal(c, "OS", lConst(lValString("Linux")));
	#else
	lDefineVal(c, "OS", lConst(lValString("*nix")));
	#endif

	#if defined(__arm__)
	lDefineVal(c, "ARCH", lConst(lValString("armv7l")));
	#elif defined(__aarch64__)
	lDefineVal(c, "ARCH", lConst(lValString("aarch64")));
	#elif defined(__x86_64__)
	lDefineVal(c, "ARCH", lConst(lValString("x86_64")));
	#elif defined(__EMSCRIPTEN__)
	lDefineVal(c, "ARCH", lConst(lValString("wasm")));
	#else
	lDefineVal(c, "ARCH", lConst(lValString("unknown")));
	#endif
}

static void lAddCoreFuncs(lClosure *c){
	lAddArithmeticFuncs(c);
	lAddBinaryFuncs(c);
	lAddPredicateFuncs(c);
	lAddBooleanFuncs(c);
	lAddCastingFuncs(c);
	lAddStringFuncs(c);
	lAddArrayFuncs(c);

	lAddNativeFunc(c,"car",     "(l)",  "Returns the car of pair l",                            lnfCar);
	lAddNativeFunc(c,"cdr",     "(l)",  "Returns the cdr of pair l",                            lnfCdr);
	lAddNativeFunc(c,"cons",    "(a b)","Returns a new pair with a as the car and b as the cdr",lnfCons);
	lAddNativeFunc(c,"set-car!","(l a)","Sets the car of pair l to a",                          lnfSetCar);
	lAddNativeFunc(c,"set-cdr!","(l a)","Sets the cdr of pair l to a",                          lnfSetCdr);

	lAddNativeFunc(c,"apply",       "(f l)",         "Evaluates f with list l as arguments",     lnfApply);
	lAddNativeFunc(c,"eval",        "(expr)",        "Evaluates expr",                           lEval);
	lAddNativeFunc(c,"read",        "(s)",           "Reads and Parses the S-Expression in S ",  lnfRead);
	lAddNativeFunc(c,"resolve",     "(s)",           "Resolves s until it is no longer a symbol",lResolve);
	lAddNativeFunc(c,"memory-info", "()",            "Returns memory usage data",                lnfMemInfo);
	lAddNativeFunc(c,"lambda lam λ \\","(args ...body)","Creates a new lambda",                  lnfLambda);
	lAddNativeFunc(c,"dynamic dyn δ",  "(args ...body)","New Dynamic scoped lambda",             lnfDynamic);
	lAddNativeFunc(c,"object obj ω",   "(args ...body)","Creates a new object",                  lnfObject);
	lAddNativeFunc(c,"self",        "()",            "Returns the closest object closure",       lnfSelf);
	lAddNativeFunc(c,"cl",          "(i)",           "Returns closure",                          lnfCl);
	lAddNativeFunc(c,"cl-lambda",   "(i)",           "Returns closure as a lambda",              lnfClLambda);
	lAddNativeFunc(c,"cl-text",     "(f)",           "Returns closures text segment",            lnfClText);
	lAddNativeFunc(c,"cl-data",     "(f)",           "Returns closures data segment",            lnfClData);
	lAddNativeFunc(c,"type-of",     "(val)",         "Return a symbol describing the type of VAL",lnfTypeOf);
	lAddNativeFunc(c,"symbol-table","(off len)",     "Returns a list of len symbols defined, accessible from the current closure from offset off",lnfSymTable);
	lAddNativeFunc(c,"symbol-count","()",            "Returns a count of the symbols accessible from the current closure",lnfSymCount);

	lAddNativeFunc(c,"if",  "(pred? then ...else)","Evalutes then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddNativeFunc(c,"cond","(...c)",              "Contains at least 1 cond block of form (pred? ...body) and evaluates and returns the first where pred? is #t",lnfCond);

	lAddNativeFunc(c,"define def","(sym val)",     "Define a new symbol SYM and link it to value VAL",                  lnfDef);
	lAddNativeFunc(c,"undefine!", "(sym)",         "Removes symbol SYM from the first symbol-table it is found in",     lnfUndef);
	lAddNativeFunc(c,"let",       "(args ...body)","Creates a new closure with args bound in which to evaluate ...body",lnfLet);
	lAddNativeFunc(c,"begin",     "(...body)",     "Evaluates ...body in order and returns the last result",            lnfBegin);
	lAddNativeFunc(c,"quote",     "(v)",           "Returns v as is without evaluating",                                lnfQuote);
	lAddNativeFunc(c,"set!",      "(s v)",         "Binds a new value v to already defined symbol s",                   lnfSet);
	lAddNativeFunc(c,"last-pair", "(l)",           "Returns the last pair of list l",                                   lnfLastPair);

	lAddNativeFunc(c,"time",       "()",         "Returns unix time",lnfTime);
	lAddNativeFunc(c,"strftime",   "(ts format)","Returns TS as a date using FORMAT (uses strftime)",lnfStrftime);
	lAddNativeFunc(c,"msecs",      "()",         "Returns monotonic msecs",lnfMsecs);
	lAddNativeFunc(c,"random",     "(&n)",       "Return a random value from 0 to &N, if &N is #nil then return a random value up to INT_MAX",lnfRandom);
	lAddNativeFunc(c,"random-seed","(seed)",     "Sets the RNG Seed to SEED",lnfRandomSeed);
}

extern unsigned char src_tmp_stdlib_nuj_data[];

lClosure *lClosureNewRoot(){
	const uint ci = lClosureAlloc();
	if(ci == 0){return NULL;}
	lClosure *c = &lClosureList[ci];
	c->parent = 0;
	c->flags |= lfNoGC;
	lAddCoreFuncs(c);
	lEval(c,lWrap(lRead((const char *)src_tmp_stdlib_nuj_data)));
	lAddPlatformVars(c);
	return c;
}

static lVal *lGetSym(uint c, lSymbol *s){
	if((c == 0) || (s == NULL)){return NULL;}
	uint sym = lvSymI(s);
	forEach(v,lCloData(c)){
		lVal *cursym = lCaar(v);
		if((cursym == NULL) || (sym != cursym->vCdr)){continue;}
		return lCdar(v);
	}
	return NULL;
}

lVal *lGetClosureSym(uint c, lSymbol *s){
	if(c == 0){return NULL;}
	lVal *t = lGetSym(c,s);
	return t != NULL ? t : lGetClosureSym(lCloParent(c),s);
}

lVal *lDefineClosureSym(uint c, lSymbol *s){
	if(c == 0){return NULL;}
	lVal *get = lGetSym(c,s);
	if(get != NULL){return get;}
	lVal *t = lCons(lValSymS(s),lCons(NULL,NULL));
	if(t == NULL){return NULL;}
	if(lCloData(c) == NULL){
		lCloData(c) = lCons(t,NULL);
	}else{
		lVal *cdr = NULL;
		for(cdr = lCloData(c);(cdr != NULL) && (lCdr(cdr) != NULL);cdr = lCdr(cdr)){}
		if(cdr == NULL){return NULL;}
		cdr->vList.cdr = lCons(t,NULL);
	}
	return t->vList.cdr;
}

lVal *lResolveSym(uint c, lVal *v){
	if((v == NULL) || (v->type != ltSymbol)){return NULL;}
	lVal *ret = lGetClosureSym(c,lvSym(v->vCdr));
	return ret == NULL ? v : lCar(ret);
}

lVal  *lApply(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *ret = NULL, *cc = NULL;

	forEach(t,v){
		lVal *ct = func(c,lCar(t));
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
static void lNFuncGCMark(lNFunc *f);

static void lValGCMark(lVal *v){
	if((v == NULL) || (v->flags & lfMarked)){return;} // Circular refs
	v->flags |= lfMarked;

	switch(v->type){
	case ltPair:
		lValGCMark(lCar(v));
		lValGCMark(lCdr(v));
		break;
	case ltLambda:
		lClosureGCMark(&lClo(v->vCdr));
		break;
	case ltArray:
		lArrayGCMark(&lArr(v));
		break;
	case ltString:
		lStrFlags(v) |= lfMarked;
		break;
	case ltVec:
		lVecFlags(v->vCdr) |= lfMarked;
		break;
	case ltNativeFunc:
		lNFuncGCMark(&lNFN(v->vCdr));
		break;
	default:
		break;
	}
}

static void lClosureGCMark(lClosure *c){
	if((c == NULL) || (c->flags & lfMarked) || (!(c->flags & lfUsed))){return;} // Circular refs
	c->flags |= lfMarked;

	lValGCMark(c->data);
	lValGCMark(c->text);
	lClosureGCMark(&lClo(c->parent));
}

static void lArrayGCMark(lArray *v){
	if((v == NULL) || (v->nextFree != 0)){return;}
	v->flags |= lfMarked;
	for(int i=0;i<v->length;i++){
		if(v->data[i] == 0){continue;}
		lValGCMark(lValD(v->data[i]));
	}
}

static void lNFuncGCMark(lNFunc *f){
	if((f == NULL) || (f->flags & lfMarked)){return;}
	f->flags |= lfMarked;
	lValGCMark(f->doc);
}

static void lGCMark(){
	for(uint i=0;i<lValMax;i++){
		if(!(lValList[i].flags & lfNoGC)){continue;}
		lValGCMark(&lValList[i]);
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
	for(uint i=0;i<lNFuncMax;i++){
		if(!(lNFuncList[i].flags & lfNoGC)){continue;}
		lNFuncGCMark(&lNFuncList[i]);
	}
	for(uint i=0;i<lNFuncMax;i++){
		if(!(lVecList[i].flags & lfNoGC)){continue;}
		lVecFlags(i) |= lfMarked;
	}
}

static void lGCSweep(){
	for(uint i=0;i<lValMax;i++){
		if(lValList[i].flags & lfMarked){
			lValList[i].flags &= ~lfMarked;
			continue;
		}
		lValFree(&lValList[i]);
	}
	for(uint i=0;i<lClosureMax;i++){
		if(lClosureList[i].flags & lfMarked){
			lClosureList[i].flags &= ~lfMarked;
			continue;
		}
		lClosureFree(i);
	}
	for(uint i=0;i<lStringMax;i++){
		if(lStringList[i].flags & lfMarked){
			lStringList[i].flags &= ~lfMarked;
			continue;
		}
		lStringFree(i);
	}
	for(uint i=0;i<lArrayMax;i++){
		if(lArrayList[i].flags & lfMarked){
			lArrayList[i].flags &= ~lfMarked;
			continue;
		}
		lArrayFree(i);
	}
	for(uint i=0;i<lNFuncMax;i++){
		if(lNFuncList[i].flags & lfMarked){
			lNFuncList[i].flags &= ~lfMarked;
			continue;
		}
		lNFuncFree(i);
	}
	for(uint i=0;i<lVecMax;i++){
		if(lVecList[i].flags & lfMarked){
			lVecList[i].flags &= ~lfMarked;
			continue;
		}
		lVecFree(i);
	}
}

static void lClosureDoGC(){
	lGCRuns++;
	lGCMark();
	lGCSweep();
}

void lClosureGC(){
	static int calls = 0;

	int thresh =         (VAL_MAX - (int)lValActive)     - (VAL_MAX / 4096);
	thresh = MIN(thresh,((CLO_MAX - (int)lClosureActive) - 128) * 4);
	thresh = MIN(thresh,((ARR_MAX - (int)lArrayActive)   -  64) * 8);
	thresh = MIN(thresh,((STR_MAX - (int)lStringActive)  -  64) * 8);
	thresh = MIN(thresh,((VEC_MAX - (int)lVecActive)     -  64) * 8);
	thresh = MIN(thresh,((SYM_MAX - (int)lSymbolActive)  -  64) * 8);
	if(++calls < thresh){return;}
	lClosureDoGC();
	calls = 0;
}

lType lTypecast(const lType a,const lType b){
	if((a == ltInf)   || (b == ltInf))  {return ltInf;}
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt))  {return ltInt;}
	if((a == ltBool)  || (b == ltBool)) {return ltBool;}
	if (a == b){ return a;}
	return ltNoAlloc;
}

lType lTypecastList(lVal *a){
	if((a == NULL) || (a->type != ltPair) || (lCar(a) == NULL)){return ltNoAlloc;}
	lType ret = lGetType(lCar(a));
	forEach(t,lCdr(a)){ret = lTypecast(ret,lGetType(lCar(t)));}
	return ret;
}

lType lGetType(lVal *v){
	return v == NULL ? ltNoAlloc : v->type;
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


lVal *getLArgB(lClosure *c, lVal *v, bool *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfBool(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = tlv->vBool;
	}
	return lCdr(v);
}

lVal *getLArgI(lClosure *c, lVal *v, int *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfInt(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = tlv->vInt;
	}
	return lCdr(v);
}

lVal *getLArgF(lClosure *c, lVal *v, float *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfFloat(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = tlv->vFloat;
	}
	return lCdr(v);
}

lVal *getLArgV(lClosure *c, lVal *v, vec *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfVec(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = lVecV(tlv->vCdr);
	}
	return lCdr(v);
}

lVal *getLArgS(lClosure *c, lVal *v,const char **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfString(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = lStrData(tlv);
	}
	return lCdr(v);
}

lVal *getLArgL(lClosure *c, lVal *v,lVal **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lEval(c,lCar(v));
	if((tlv != NULL) && ((tlv->type == ltLambda) || (tlv->type == ltNativeFunc))){
		*res = tlv;
	}
	return lCdr(v);
}

lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}

lVal *lWrap(lVal *v){
	return lCons(lValSymS(symBegin),v);
}

lVal *lEvalCast(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	return lCast(c,t,lTypecastList(t));
}

lVal *lEvalCastSpecific(lClosure *c, lVal *v, const lType type){
	return lCast(c,lApply(c,v,lEval),type);
}

lVal *lEvalCastNumeric(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	lType type = lTypecastList(t);
	if(type == ltString){type = ltFloat;}
	return lCast(c,t,type);
}

lVal *lLastCar(lVal *v){
	forEach(a,v){
		if(lCdr(a) == NULL){return lCar(a);}
	}
	return NULL;
}

lVal *lCar(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}

lVal *lCdr(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.cdr : NULL;
}

lVal *lCaar(lVal *v){
	return lCar(lCar(v));
}

lVal *lCadr(lVal *v){
	return lCar(lCdr(v));
}

lVal *lCdar(lVal *v){
	return lCdr(lCar(v));
}

lVal *lCddr(lVal *v){
	return lCdr(lCdr(v));
}

lVal *lCadar(lVal *v){
	return lCar(lCdr(lCar(v)));
}

lVal *lCaddr(lVal *v){
	return lCar(lCdr(lCdr(v)));
}

lVal *lCdddr(lVal *v){
	return lCdr(lCdr(lCdr(v)));
}

int lListLength(lVal *v){
	int i = 0;
	for(lVal *n = v;(n != NULL) && (lCar(n) != NULL); n = lCdr(n)){i++;}
	return i;
}

int lStringLength(const lString *s){
	return s->bufEnd - s->buf;
}

int lSymCmp(const lVal *a,const lVal *b){
	if((a == NULL) || (b == NULL)){return 2;}
	if((a->type != ltSymbol) || (b->type != ltSymbol) || (a->vCdr == 0)){return 2;}
	return a->vCdr == b->vCdr ? 0 : -1;
}

int lSymEq(const lSymbol *a,const lSymbol *b){
	return a == b ? 0 : -1;
}

lVal *lConst(lVal *v){
	v->flags |= lfConst;
	return v;
}
