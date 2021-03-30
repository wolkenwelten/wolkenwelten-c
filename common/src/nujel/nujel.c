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

#include "../tmp/assets.h"

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

char dispWriteBuf[1<<16];
lSymbol symQuote,symArr;


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

	strncpy(symQuote.c,"quote",15);
	strncpy(symArr.c,"arr",15);
}

static void lVecFree(uint i){
	if((i == 0) || (i >= lVecMax)){return;}
	lVec *v = &lVecList[i];
	if(v->nextFree != 0){return;}
	lVecActive--;
	v->nextFree   = lVecFFree;
	v->flags      = 0;
	lClosureFFree = i;
}

static uint lVecAlloc(){
	lVec *ret;
	if(lVecFFree == 0){
		if(lVecMax >= VEC_MAX-1){
			lPrintError("lVec OOM\n");
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
	lClosureFFree = i;
}

static uint lNFuncAlloc(){
	lNFunc *ret;
	if(lNFuncFFree == 0){
		if(lNFuncMax >= NFN_MAX-1){
			lPrintError("lNFunc OOM\n");
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
			lPrintError("lClosure OOM\n");
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
			lPrintError("lArray OOM\n");
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
	lArrayList[v].nextFree = lArrayFFree;
	lArrayFFree = v;
}

u32 lStringAlloc(){
	lString *ret;
	if(lStringFFree == 0){
		if(lStringMax >= STR_MAX){
			lPrintError("lString OOM\n");
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
	s->buf = s->data = nbuf;
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
			lPrintError("lVal OOM\n");
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

void lValFree(lVal *v){
	if((v == NULL) || (v->type == ltNoAlloc)){return;}
	if(v->type == ltLambda){
		lClo(v->vCdr).refCount--;
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
	lSDisplayVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)]);
	printf("%s",dispWriteBuf);
}

void lWriteVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)]);
	printf("%s\n",dispWriteBuf);
}

lVal *lMatchClosureSym(uint c, lVal *ret, const lSymbol s){
	if(c == 0){return ret;}
	const uint len = strnlen(s.c,16);

	forEach(n,lCloData(c)){
		if (n == NULL)                   {continue;}
		if (n->type != ltPair)           {continue;}
		if (n->vList.car == NULL)        {continue;}
		if (n->vList.car->type != ltPair){continue;}
		if (n->vList.car->vList.car == NULL){continue;}
		if (n->vList.car->vList.car->type != ltSymbol){continue;}
		if(strncmp(s.c,n->vList.car->vList.car->vSymbol.c,len) == 0){
			ret = lCons(n->vList.car->vList.car,ret);
		}
	}

	return lMatchClosureSym(lCloParent(c),ret,s);
}

static lVal *lnfMatchClosureSym(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = v->vList.car;
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	return lMatchClosureSym(c - lClosureList,NULL,sym->vSymbol);
}

static lVal *lnfDefine(uint c, lClosure *ec, lVal *v, lVal *(*func)(uint ,lSymbol)){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = v->vList.car;
	lVal *nv = NULL;
	if(v->vList.cdr != NULL){
		nv = lEval(ec,v->vList.cdr->vList.car);
	}
	if(sym->type != ltSymbol){sym = lEval(&lClo(c),sym);}
	if(sym->type != ltSymbol){return NULL;}
	lVal *t = func(c,sym->vSymbol);
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	if((t->vList.car != NULL) && (t->vList.car->flags & lfConst)){
		return t->vList.car;
	}else{
		return t->vList.car = nv;
	}
}

static lVal *lUndefineClosureSym(uint c, lVal *s){
	if(c == 0){return lValBool(false);}

	lVal *lastPair = lCloData(c);
	forEach(v,lCloData(c)){
		const lVal *n = v->vList.car;
		if((n == NULL) || (n->type != ltPair))  {break;}
		const lVal *sym = n->vList.car;
		if(lSymCmp(s,sym) == 0){
			lastPair->vList.cdr = v->vList.cdr;
			return lValBool(true);
		}
		lastPair = v;
	}
	return lUndefineClosureSym(lCloParent(c),s);
}
static lVal *lnfUndef(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = v->vList.car;
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
		if (n == NULL)                   {continue;}
		if (n->type != ltPair)           {continue;}
		if (n->vList.car == NULL)        {continue;}
		if (n->vList.car->type != ltPair){continue;}
		if (n->vList.car->vList.cdr == NULL){continue;}
		if (n->vList.car->vList.cdr->type != ltPair){continue;}
		if (n->vList.car->vList.cdr->vList.car == NULL){continue;}
		if((n->vList.car->vList.cdr->vList.car->type != ltNativeFunc) &&
		   (n->vList.car->vList.cdr->vList.car->type != ltLambda)){continue;}

		if(off > 0){--off; continue;}
		v = lCons(n->vList.car->vList.car,v);
		if(--len <= 0){return v;}
	}
	return lSymTable(&lClo(c->parent),v,off,len);
}

static lVal *lnfSymTable(lClosure *c, lVal *v){
	lVal *loff = lnfInt(c,lEval(c,lCarOrV(v)));
	lVal *llen = lnfInt(c,lEval(c,lCarOrV( v == NULL ? NULL : v->vList.cdr)));
	int off = loff->vInt;
	int len = llen->vInt;
	if(len <= 0){len = 1<<30;}
	return lSymTable(c,NULL,off,len);
}

static int lSymCount(lClosure *c, int ret){
	if(c == NULL){return ret;}
	forEach(n,c->data){
		if (n == NULL)                   {continue;}
		if (n->type != ltPair)           {continue;}
		if (n->vList.car == NULL)        {continue;}
		if (n->vList.car->type != ltPair){continue;}
		if (n->vList.car->vList.cdr == NULL){continue;}
		if (n->vList.car->vList.cdr->type != ltPair){continue;}
		if (n->vList.car->vList.cdr->vList.car == NULL){continue;}
		if((n->vList.car->vList.cdr->vList.car->type != ltNativeFunc) &&
		   (n->vList.car->vList.cdr->vList.car->type != ltLambda)){continue;}

		++ret;
	}
	return lSymCount(&lClo(c->parent),ret);
}

static lVal *lnfSymCount(lClosure *c, lVal *v){
	(void)v;
	return lValInt(lSymCount(c,0));
}

lVal *lnfBegin(lClosure *c, lVal *v){
	lVal *ret = NULL;
	forEach(n,v){
		ret = lEval(c,n->vList.car);
	}
	return ret;
}

static lVal *lnfCl(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfCl(&lClo(c->parent),lCons(lValInt(t->vInt - 1),NULL));
	}
	return c->data != NULL ? c->data : lCons(NULL,NULL);
}

static lVal *lnfClText(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCloText(t->vCdr);
	}else if(t->type == ltNativeFunc){
		return lCons(lNFN(t->vCdr).doc->vList.cdr,NULL);
	}
	return NULL;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCloData(t->vCdr);
	}else if(t->type == ltNativeFunc){
		return lNFN(t->vCdr).doc->vList.car;
	}
	return NULL;
}

static lVal *lnfClLambda(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,v->vList.car));
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
	if((v == NULL) || (v->vList.car == NULL) || (v->vList.cdr == NULL)){return NULL;}
	lCloText(cli) = v->vList.cdr;
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vCdr = cli;

	forEach(n,v->vList.car){
		if(n->vList.car->type != ltSymbol){continue;}
		lVal *t = lDefineClosureSym(cli,n->vList.car->vSymbol);
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

	if((v != NULL) && (v->type == ltPair)){
		forEach(n,v->vList.car){
			if(n->vList.car->type != ltSymbol){continue;}
			lVal *t = lnfDefine(cli,&lClo(cli),n,lDefineClosureSym);
			t->vList.car = NULL;
			(void)t;
		}
		lnfBegin(&lClo(cli),v->vList.cdr);
	}

	return ret;
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	return v->vList.car;
}

static lVal *lnfMem(lClosure *c, lVal *v){
	(void)c; (void)v;

	char buf[1024];
	snprintf(buf,sizeof(buf),"Vals:%u Closures:%u Arrs:%u Strings:%u NFuncs:%u",lValActive,lClosureActive,lArrayActive,lStringActive,lNFuncActive);
	fprintf(stderr,"%s\n",buf);
	return lValString(buf);
}

static lVal *lnfMemCount(lClosure *c, lVal *v){
	(void)c; (void)v;

	char buf[1024];
	int vals=0,clos=0,arrs=0,strs=0,nfuncs=0;
	for(uint i=0;i<VAL_MAX;i++){
		if(lValList[i].type == ltNoAlloc){continue;}
		vals++;
	}
	for(uint i=0;i<CLO_MAX;i++){
		if(lClosureList[i].flags & lfUsed){clos++;}
	}
	for(uint i=0;i<ARR_MAX;i++){
		if(lArrayList[i].nextFree != 0){continue;}
		arrs++;
	}
	for(uint i=0;i<STR_MAX;i++){
		if(lStringList[i].nextFree != 0){continue;}
		strs++;
	}
	for(uint i=0;i<NFN_MAX;i++){
		if(lNFuncList[i].nextFree != 0){continue;}
		nfuncs++;
	}
	snprintf(buf,sizeof(buf),"Vals:%u Closures:%u Arrs:%u Strings:%u NFuncs:%u",vals,clos,arrs,strs,nfuncs);
	fprintf(stderr,"%s\n",buf);
	return lValString(buf);
}

static lVal *lnfLet(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	const uint nci = lClosureNew(c - lClosureList);
	if(nci == 0){return NULL;}
	lClosure *nc = &lClosureList[nci & CLO_MASK];
	forEach(n,v->vList.car){
		lnfDefine(nci,c,n->vList.car,lDefineClosureSym);
	}
	lVal *ret = NULL;
	forEach(n,v->vList.cdr){
		ret = lEval(nc,n->vList.car);
	}
	c->refCount--;
	return ret == NULL ? NULL : ret;
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
	if(lambda->flags & lfObject){
		return lnfBegin(lambda,v);
	}
	lVal *vn = v;
	uint tmpci = 0;
	if(lambda->flags & lfDynamic){
		tmpci = lClosureNew(c - lClosureList);
	}else{
		tmpci = lClosureNew(lambda - lClosureList);
	}
	if(tmpci == 0){return NULL;}
	lClosure *tmpc = &lClosureList[tmpci & CLO_MASK];
	tmpc->text = lambda->text;
	forEach(n,lambda->data){
		if(vn == NULL){break;}
		lVal *nn = n->vList.car;
		if(nn->type != ltPair)             {continue;}
		if(nn->vList.car == NULL)          {continue;}
		if(nn->vList.car->type != ltSymbol){continue;}
		const lSymbol csym = nn->vList.car->vSymbol;
		lVal *lv = lDefineClosureSym(tmpci,csym);
		if(lSymVariadic(csym)){
			lVal *t = lSymNoEval(csym) ? vn : lApply(c,vn,lEval);
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			break;
		}else{
			lVal *t = lSymNoEval(csym) ? vn->vList.car : lEval(c,vn->vList.car);
			if(t  != NULL && t->type == ltSymbol && !lSymNoEval(csym)){t = lEval(c,t);}
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			if(vn != NULL){vn = vn->vList.cdr;}
		}
	}

	lVal *ret = NULL;
	forEach(n,lambda->text){
		ret = lEval(tmpc,n->vList.car);
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
static lVal *lnfSetCar(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrN(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *car = NULL;
	if((v != NULL) && (v->type == ltPair) && (v->vList.cdr != NULL)){car = lEval(c,lCarOrN(v->vList.cdr));}
	t->vList.car = car;
	return t;
}
static lVal *lnfSetCdr(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrN(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *cdr = NULL;
	if((v != NULL) && (v->type == ltPair) && (v->vList.cdr != NULL)){cdr = lEval(c,lCarOrN(v->vList.cdr));}
	t->vList.cdr = cdr;
	return t;
}

static lVal *lnfLastPair(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrN(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	for(lVal *r=t;r != NULL;r = r->vList.cdr){
		if(r->vList.cdr == NULL){return r;}
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
	v = lEval(c,lCarOrV(v));
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
		lVal *ret = lEval(c,v->vList.car);
		if(ret == NULL){return v;}
		switch(ret->type){
		default:
			return v;
		case ltNativeFunc:
			return lNFN(ret->vCdr).fp(c,v->vList.cdr);
		case ltLambda:
			return lLambda(c,v->vList.cdr,&lClo(ret->vCdr));
		case ltPair:
			return lEval(c,ret);
		case ltString:
			return lnfCat(c,v);
		case ltArray:
			return lnfArrRS(c,v);
		}
	}
	return v;
}

lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lEval(c,lCarOrV(v));
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c - lClosureList,func);}
	switch(func->type){
	case ltNativeFunc:
		if(lNFN(func->vCdr).fp == NULL){return v;}
		return lNFN(func->vCdr).fp(c,lEval(c,lCarOrV(v->vList.cdr)));
	case ltLambda: {
		lVal *t = lCarOrV(v->vList.cdr);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,&lClo(func->vCdr));}
	default:
		return v;
	}
}

void lDefineVal(lClosure *c, const char *sym, lVal *val){
	lSymbol S;
	memset(S.c,0,sizeof(S.c));
	snprintf(S.c,sizeof(S.c),"%s",sym);

	lVal *var = lDefineClosureSym(c - lClosureList,S);
	if(var == NULL){return;}
	var->vList.car = val;
}

lVal *lnfRead(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	t = lReadString(&lStr(t));
	if((t != NULL) && (t->type == ltPair) && (t->vList.car != NULL) && (t->vList.cdr == NULL)){
		return t->vList.car;
	}else{
		return t;
	}
}

void lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lSymbol S;
	memset(S.c,0,sizeof(S.c));
	snprintf(S.c,sizeof(S.c),"%s",sym);

	lVal *var = lDefineClosureSym(c - lClosureList,S);
	if(var == NULL){
		lPrintError("Error adding NFunc %s\n",sym);
		return;
	}
	lVal *lArgs = lRead(args);
	var->vList.car = lValNativeFunc(func,lArgs,lValString(doc));
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
	lAddNativeFunc(c,"match-sym",   "(s)",           "Returns all symbols partially matching s", lnfMatchClosureSym);
	lAddNativeFunc(c,"mem",         "()",            "Returns memory usage data",                lnfMem);
	lAddNativeFunc(c,"mem-count",   "()",            "Returns memory usage data by counting",    lnfMemCount);
	lAddNativeFunc(c,"λ",           "(args ...body)","Creates a new lambda",                     lnfLambda);
	lAddNativeFunc(c,"lambda",      "(args ...body)","New Lambda",                               lnfLambda);
	lAddNativeFunc(c,"δ",           "(args ...body)","New Dynamic scoped lambda",                lnfDynamic);
	lAddNativeFunc(c,"dynamic",     "(args ...body)","New Dynamic scoped lambda",                lnfDynamic);
	lAddNativeFunc(c,"ω",           "(args ...body)","Creates a new object",                     lnfObject);
	lAddNativeFunc(c,"object",      "(args ...body)","Creates a new object",                     lnfObject);
	lAddNativeFunc(c,"cl",          "(i)",           "Returns closure",                          lnfCl);
	lAddNativeFunc(c,"cl-lambda",   "(i)",           "Returns closure as a lambda",              lnfClLambda);
	lAddNativeFunc(c,"cl-text",     "(f)",           "Returns closures text segment",            lnfClText);
	lAddNativeFunc(c,"cl-data",     "(f)",           "Returns closures data segment",            lnfClData);
	lAddNativeFunc(c,"symbol-table","(off len)",     "Returns a list of len symbols defined, accessible from the current closure from offset off",lnfSymTable);
	lAddNativeFunc(c,"symbol-count","()",            "Returns a count of the symbols accessible from the current closure",lnfSymCount);

	lAddNativeFunc(c,"if",  "(pred? then ...else)","Evalutes then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddNativeFunc(c,"cond","(...c)",              "Contains at least 1 cond block of form (pred? ...body) and evaluates and returns the first where pred? is #t",lnfCond);

	lAddNativeFunc(c,"def",      "(sym val)",         "Define a new symbol SYM and link it to value VAL",              lnfDef);
	lAddNativeFunc(c,"define",   "(sym val)",         "Define a new symbol SYM and link it to value VAL",              lnfDef);
	lAddNativeFunc(c,"undefine!","(sym)",           "Removes symbol SYM from the first symbol-table it is found in",   lnfUndef);
	lAddNativeFunc(c,"let",      "(args ...body)","Creates a new closure with args bound in which to evaluate ...body",lnfLet);
	lAddNativeFunc(c,"begin",    "(...body)",     "Evaluates ...body in order and returns the last result",            lnfBegin);
	lAddNativeFunc(c,"quote",    "(v)",           "Returns v as is without evaluating",                                lnfQuote);
	lAddNativeFunc(c,"set!",     "(s v)",         "Binds a new value v to already defined symbol s",                   lnfSet);
	lAddNativeFunc(c,"last-pair","(l)",           "Returns the last pair of list l",                                   lnfLastPair);

	lAddNativeFunc(c,"abs","(a)","Returns the absolute value of a",     lnfAbs);
	lAddNativeFunc(c,"pow","(a b)","Returns a raised to the power of b",lnfPow);
	lAddNativeFunc(c,"sqrt","(a)","Returns the squareroot of a",        lnfSqrt);
	lAddNativeFunc(c,"floor","(a)","Rounds a down",                     lnfFloor);
	lAddNativeFunc(c,"ceil","(a)","Rounds a up",                        lnfCeil);
	lAddNativeFunc(c,"round","(a)","Rounds a",                          lnfRound);

	lAddNativeFunc(c,"vx","(v)","Returns x part of vector v",lnfVX);
	lAddNativeFunc(c,"vy","(v)","Returns x part of vector v",lnfVY);
	lAddNativeFunc(c,"vz","(v)","Returns x part of vector v",lnfVZ);

	lAddNativeFunc(c,"msecs", "()","Returns monotonic msecs",lnfMsecs);
	lAddNativeFunc(c,"random","(&n)","Retur a random value from 0 to &N, if &N is #nil then return a random value up to INT_MAX",lnfRandom);
	lAddNativeFunc(c,"random-seed","(seed)","Sets the RNG Seed to SEED",lnfRandomSeed);

	lDefineVal(c,"π",  lConst(lValFloat(PI)));
	lDefineVal(c,"PI", lConst(lValFloat(PI)));
}

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

static lVal *lGetSym(uint c, const lSymbol s){
	if(c == 0){return NULL;}
	forEach(v,lCloData(c)){
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

lVal *lGetClosureSym(uint c, const lSymbol s){
	if(c == 0){return NULL;}
	lVal *t = lGetSym(c,s);
	return t != NULL ? t : lGetClosureSym(lCloParent(c),s);
}

lVal *lDefineClosureSym(uint c,const lSymbol s){
	if(c == 0){return NULL;}
	lVal *get = lGetSym(c,s);
	if(get != NULL){return get;}
	lVal *t = lCons(lValSymS(s),lCons(NULL,NULL));
	if(t == NULL){return NULL;}
	if(lCloData(c) == NULL){
		lCloData(c) = lCons(t,NULL);
	}else{
		lVal *cdr = NULL;
		for(cdr = lCloData(c);(cdr != NULL) && (cdr->vList.cdr != NULL);cdr = cdr->vList.cdr){}
		if(cdr == NULL){return NULL;}
		cdr->vList.cdr = lCons(t,NULL);
	}
	return t->vList.cdr;
}

lVal *lResolveSym(uint c, lVal *v){
	if((v == NULL) || (v->type != ltSymbol)){return NULL;}
	lVal *ret = lGetClosureSym(c,v->vSymbol);
	return ret == NULL ? v : ret->vList.car;
}

lVal  *lApply(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if((c == NULL) || (v == NULL)){return NULL;}
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
static void lNFuncGCMark(lNFunc *f);

static void lValGCMark(lVal *v){
	if((v == NULL) || (v->flags & lfMarked)){return;} // Circular refs
	v->flags |= lfMarked;

	switch(v->type){
	case ltPair:
		lValGCMark(v->vList.car);
		lValGCMark(v->vList.cdr);
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
		lValGCMark(v->data[i]);
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

	int thresh = (VAL_MAX - (int)lValActive)-4096;
	thresh = MIN(thresh,((CLO_MAX - (int)lClosureActive)-128)*4);
	thresh = MIN(thresh,((ARR_MAX - (int)lArrayActive)-64)*8);
	thresh = MIN(thresh,((STR_MAX - (int)lStringActive)-64)*8);
	if(++calls < thresh){return;}
	lClosureDoGC();
	calls = 0;
}

lType lTypecast(const lType a,const lType b){
	if((a == ltInf)   || (b == ltInf))  {return ltInf;}
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt)  ){return ltInt;}
	if((a == ltBool)  || (b == ltBool) ){return ltBool;}
	if (a == b){ return a;}
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


lVal *getLArgB(lClosure *c, lVal *v, bool *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfBool(c,lEval(c,v->vList.car));
	if(tlv != NULL){
		*res = tlv->vBool;
	}
	return v->vList.cdr;
}

lVal *getLArgI(lClosure *c, lVal *v, int *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfInt(c,lEval(c,v->vList.car));
	if(tlv != NULL){
		*res = tlv->vInt;
	}
	return v->vList.cdr;
}

lVal *getLArgF(lClosure *c, lVal *v, float *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfFloat(c,lEval(c,v->vList.car));
	if(tlv != NULL){
		*res = tlv->vFloat;
	}
	return v->vList.cdr;
}

lVal *getLArgV(lClosure *c, lVal *v, vec *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfVec(c,lEval(c,v->vList.car));
	if(tlv != NULL){
		*res = lVecV(tlv->vCdr);
	}
	return v->vList.cdr;
}

lVal *getLArgS(lClosure *c, lVal *v,const char **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfString(c,lEval(c,v->vList.car));
	if(tlv != NULL){
		*res = lStrData(tlv);
	}
	return v->vList.cdr;
}
