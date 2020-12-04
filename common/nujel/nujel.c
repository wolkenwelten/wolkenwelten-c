#include "nujel.h"
#include "common.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lVal      lValList[1<<12];
uint      lValMax = 0;
lVal     *lValFFree = NULL;

lClosure  lClosureList[1<<6];
lClosure *lClosureFFree = NULL;

lString   lStringList[1<<8];
lString  *lStringFFree = NULL;

lClosure *lClosureAlloc();
void      lClosureFree(lClosure *c);
lVal     *lValAlloc();
void      lValFree(lVal *v);
lString  *lStringAlloc();
void      lStringFree(lString *s);

static lVal *lDefineClosureSym(lClosure *c,const lSymbol s);
static lVal *lResolveClosureSymbol(lClosure *c, const lSymbol s);

void lInit(){
	for(uint i=0;i<countof(lValList)-1;i++){
		lValList[i].type = ltNoAlloc;
		lValList[i].next = &lValList[i+1];
	}
	lValFFree = &lValList[0];
	for(uint i=0;i<countof(lClosureList)-1;i++){
		lClosureList[i].parent = &lClosureList[i+1];
	}
	lClosureFFree = &lClosureList[0];
	for(uint i=0;i<countof(lStringList)-1;i++){
		lStringList[i].next = &lStringList[i+1];
	}
	lStringFFree = &lStringList[0];
}

lClosure *lClosureAlloc(){
	if(lClosureFFree == NULL){
		fprintf(stderr,"lClosure OOM\n");
		return NULL;
	}
	lClosure *ret = lClosureFFree;
	lClosureFFree = ret->parent;
	return ret;
}
void lClosureFree(lClosure *c){
	c->parent = lClosureFFree;
	lClosureFFree = c;
	lValFree(c->data);
}

lString *lStringAlloc(){
	if(lStringFFree == NULL){
		fprintf(stderr,"lString OOM\n");
		return NULL;
	}
	lString *ret = lStringFFree;
	lStringFFree = ret->next;
	return ret;
}
void lStringFree(lString *s){
	if(s == NULL){return;}
	if(s->buf == NULL){return;}
	free(s->buf);
	s->buf = s->data = NULL;
	s->next = lStringFFree;
}

lVal *lValAlloc(){
	if(lValFFree == NULL){
		fprintf(stderr,"lVal OOM\n");
		return NULL;
	}
	lVal *ret  = lValFFree;
	lValFFree  = ret->next;
	ret->next  = NULL;
	ret->flags = 0;
	lValMax = MAX(lValMax,(uint)(ret-lValList));
	return ret;
}
void lValFree(lVal *v){
	if(v == NULL){return;}
	switch(v->type){
	case ltString:
		lStringFree(v->vString);
		break;
	case ltClosure:
		lClosureFree(v->vClosure);
		break;
	default:
		break;
	}
	v->type   = ltNoAlloc;
	v->next   = lValFFree;
	v->flags  = 0;
	lValFFree = v;
}
lVal *lValCopy(const lVal *v){
	lVal *ret = lValAlloc();
	*ret = *v;
	ret->next = NULL;
	return ret;
}

lVal *lValNil(){
	lVal *ret = lValAlloc();
	ret->type = ltNil;
	return ret;
}
lVal *lValInf(){
	lVal *ret = lValAlloc();
	ret->type = ltInf;
	return ret;
}
lVal *lValInt(int v){
	lVal *ret = lValAlloc();
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}
lVal *lValSym(const char *s){
	lVal *ret = lValAlloc();
	ret->type = ltSymbol;
	ret->vSymbol.v = 0;
	strncpy(ret->vSymbol.c,s,8);
	return ret;
}

lClosure *lClosureNew(lClosure *parent){
	lClosure *c = lClosureAlloc();
	if(c == NULL){
		return NULL;
	}
	c->parent = parent;
	c->data = lValAlloc();
	c->data->type = ltNil;
	return c;
}
/*
static lVal *lStringNew(const char *str){
	size_t len = strlen(str);
	lString *s = lStringAlloc();
	s->buf = s->data = malloc(len+1)
	memcpy(s->data,str,len)
	s->data[len] = 0;
	s->len       = len;
	s->bufEnd    = &s->buf[len+1];
	return s;
}
*/
static void lStringAdvanceToNextCharacter(lString *s){
	for(;(*s->data != 0) && (isspace(*s->data));s->data++){}
}

static lVal *lParseString(lString *s){
	static char buf[4096];
	char *b = buf;
	for(uint i=0;i<sizeof(buf);i++){
		if(*s->data == '\\'){
			s->data++;
			switch(*s->data){
			case '\\':
				*b++ = '\\';
				break;
			case '"':
				*b++ = '"';
				break;
			case 'n':
				*b++ = '\n';
				break;
			case 'r':
				*b++ = '\r';
				break;
			case 't':
				*b++ = '\t';
				break;
			case '0':
				*b++ = 0;
				break;
			}
			s->data++;
		}else if(*s->data == '"'){
			s->data++;
			lVal *v = lValAlloc();
			v->type = ltString;
			lString *s = v->vString = lStringAlloc();
			s->data = s->buf = malloc((b-buf)+1);
			memcpy(s->buf,buf,b-buf);
			s->len = b-buf;
			s->data[s->len] = 0;
			s->bufEnd = s->data + s->len;
			return v;
		}else{
			*b++ = *s->data++;
		}
	}
	lVal *v = lValAlloc();
	v->type = ltNil;
	return v;
}

static lVal *lParseNumber(lString *s){
	lVal *v = lValAlloc();
	v->type = ltInt;
	v->vInt = 0;
	int c   = *s->data;
	int fc  = c;
	while(!isspace(c)){
		if(c == 0){break;}
		if(isdigit(c)){
			v->vInt *= 10;
			v->vInt += c - '0';
		}else if((c != ',') && (c != '.') && (c != '_')){
			break;
		}
		c = *++s->data;
	}
	if(fc == '-'){
		v->vInt = -v->vInt;
	}
	//printf("parseNumber: %u\n",v->vInt);
	return v;
}

static lVal *lParseSymbol(lString *s){
	lVal *v = lValAlloc();
	v->type = ltSymbol;
	v->vSymbol.v = 0;
	for(int i=0;i<8;i++){
		char c = *s->data++;
		if(isspace(c) || (c == ')') || (c ==0)){
			s->data--;
			break;
		}
		v->vSymbol.c[i] = c;
	}
	while(isspace(*s->data)){
		if(*s->data == 0){break;}
		s->data++;
	}
	return v;
}

static lVal *lParseSpecial(lString *s){
	lVal *v = lValAlloc();
	v->type = ltNil;
	if(*s->data++ != '#'){return v;}
	switch(*s->data++){
	default:
	case 'n': return v;
	case 't':
		v->type  = ltBool;
		v->vBool = true;
		return v;
	case 'f':
		v->type  = ltBool;
		v->vBool = false;
		return v;
	case 'i':
		v->type  = ltInf;
		return v;
	case 'z':
		v->type  = ltNoAlloc;
		return v;
	case 'c':
		v->type  = ltNativeFunc;
		return v;
	}

}

static lVal *lParse(lString *s){
	lVal *v = NULL, *ret = NULL;
	while(1){
		lStringAdvanceToNextCharacter(s);
		char c = *s->data;
		//printf("Parse {%c} %u\n",c,c);
		lVal *t = NULL;
		if((c == 0) || (c == ')') || (s->data >= s->bufEnd)){
			if(v == NULL){
				v = lValAlloc();
				v->type = ltNil;
			}
			if(ret == NULL){ret = v;}
			s->data++;
			return ret;
		}else if(c == '('){
			t = lValAlloc();
			t->type = ltLambda;
			s->data+=1;
			t->vList = lParse(s);
		}else if((c == '\'') && (s->data[1] == '(')){
			t = lValAlloc();
			t->type = ltList;
			s->data+=2;
			t->vList = lParse(s);
		}else if(c == '"'){
			s->data++;
			t = lParseString(s);
		}else if(isdigit(c)){
			t = lParseNumber(s);
		}else if(c == '#'){
			t = lParseSpecial(s);
		}else if((c == '-') && (isdigit(*s->data))){
			t = lParseNumber(s);
		}else{
			t = lParseSymbol(s);
		}
		if(v != NULL){v->next = t;}
		if(ret == NULL){ret = t;}
		v = t;
	}
}

lVal *lParseSExprCS(char *str){
	lString *s = lStringAlloc();
	s->data    = str;
	s->len     = strlen(str);
	s->bufEnd  = str+s->len+1;
	lVal *ret  = lParse(s);
	lStringFree(s);
	return ret;
}

void lPrintChain(lVal *v){
	static char buf[8192];
	lSPrintChain(v,buf,&buf[sizeof(buf) - 1]);
	printf("%s",buf);
}

char *lSPrintVal(lVal *v, char *buf, char *bufEnd){
	char *cur = buf;
	int t = 0;
	int len = bufEnd-buf;
	if(v == NULL){
		t = snprintf(buf,len,"NULL");
	}else{
		switch(v->type){
		case ltNoAlloc:
			t = snprintf(buf,len,"#zzz");
			break;
		case ltNil:
			t = snprintf(buf,len,"#nil");
			break;
		case ltBool:
			if(v->vBool){
				t = snprintf(buf,len,"#t");
			}else{
				t = snprintf(buf,len,"#f");
			}
			break;
		case ltList: {
			t = snprintf(buf,bufEnd-cur,"(");
			if(t > 0){cur += t;}
			for(lVal *n = v->vList;n != NULL;n = n->next){
				cur = lSPrintVal(n,cur,bufEnd);
			}
			t = snprintf(buf,bufEnd-cur,")");
			break; }
		case ltLambda: {
			t = snprintf(buf,bufEnd-cur,"(");
			if(t > 0){cur += t;}
			for(lVal *n = v->vList;n != NULL;n = n->next){
				cur = lSPrintVal(n,cur,bufEnd);
			}
			t = snprintf(buf,bufEnd-cur,")");
			break; }
		case ltClosure:
			t = snprintf(buf,len,"#{");
			break;
		case ltInt:
			t = snprintf(buf,len,"%i",v->vInt);
			break;
		case ltString:
			t = snprintf(buf,len,"\"%s\"",v->vString->data);
			break;
		case ltSymbol:
			t = snprintf(buf,len,"%.8s",v->vSymbol.c);
			break;
		case ltNativeFunc:
			t = snprintf(buf,len,"#cfn");
			break;
		case ltInf:
			t = snprintf(buf,len,"#inf");
			break;
		}
	}
	if(t > 0){cur += t;}
	return cur;
}

char *lSPrintChain(lVal *start, char *buf, char *bufEnd){
	char *cur = buf;
	for(lVal *v = start;v!=NULL;v = v->next){
		cur = lSPrintVal(v,cur,bufEnd);
		if((bufEnd - cur) < 1){return cur;}
		*cur++ = ' ';
	}
	if((bufEnd - cur) < 1){return cur;}
	*cur++ = '\n';
	*bufEnd = 0;
	return cur;
}

lVal *lnfAdd(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValCopy(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltInt){continue;}
		t->vInt += rv->vInt;
	}
	return t;
}

lVal *lnfSub(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValCopy(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltInt){continue;}
		t->vInt -= rv->vInt;
	}
	return t;
}

lVal *lnfMul(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValCopy(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type == ltInf){return rv;}
		if(rv->type != ltInt){continue;}
		t->vInt *= rv->vInt;
	}
	return t;
}

lVal *lnfDiv(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValCopy(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type != ltInt){continue;}
		if(rv->vInt == 0){return lValInf();}
		t->vInt /= rv->vInt;
	}
	return t;
}

lVal *lnfMod(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t == NULL){return NULL;}
	t = lValCopy(t);
	for(lVal *r = v->next;r != NULL;r = r->next){
		lVal *rv = lEval(c,r);
		if(rv->type != ltInt){continue;}
		if(rv->vInt == 0){return lValInf();}
		t->vInt %= rv->vInt;
	}
	return t;
}

static lVal *lnfDef(lClosure *c, lVal *v){
	if(v == NULL)                {return lValNil();}
	lVal *sym = v;
	if(sym->type != ltSymbol)    {sym = lEval(c,sym);}
	if(sym->type != ltSymbol)    {return lValNil();}
	lVal *t = lDefineClosureSym(c,sym->vSymbol);
	if(t == NULL)                {return lValNil();}
	lVal *val = lEval(c,v->next);
	if(val != NULL){*t = *val;}
	return t;
}

static lVal *lnfSet(lClosure *c, lVal *v){
	if(v == NULL)                {return lValNil();}
	lVal *sym = v;
	if(sym->type != ltSymbol)    {sym = lEval(c,sym);}
	if(sym->type != ltSymbol)    {return lValNil();}
	lVal *t = lResolveClosureSymbol(c,sym->vSymbol);
	if(t == NULL)                {return lValNil();}
	lVal *val = lEval(c,v->next);
	if(val != NULL){*t = *val;}
	return t;
}

static lVal *lnfCl(lClosure *c, lVal *v){
	if(c == NULL)       {return lValNil();}
	if(v == NULL)       {return c->data;}
	if(v->next == NULL) {return c->data;}
	lVal *t = v->next;
	if(t->type == ltSymbol){t = lResolveSym(c,t->vSymbol);}
	if(t->type == ltInt){
		if(t->vInt > 0){
			t = lValCopy(t);
			t->vInt--;
			return lnfCl(c->parent,v);
		}
	}
	return c->data;
}

static lVal *lnfMem(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInt(lMemUsage());
}

static lVal *lValNativeFunc(lVal *(*func)(lClosure *,lVal *)){
	lVal *v = lValAlloc();
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

static lVal *lResolveNativeSym(const lSymbol s){
	switch(s.c[0]){
	case '+': return lValNativeFunc(lnfAdd);
	case '-': return lValNativeFunc(lnfSub);
	case '*': return lValNativeFunc(lnfMul);
	case '/': return lValNativeFunc(lnfDiv);
	case '%': return lValNativeFunc(lnfMod);
	}
	if(strcmp(s.c,"def") == 0)  {return lValNativeFunc(lnfDef);}
	if(strcmp(s.c,"set") == 0)  {return lValNativeFunc(lnfSet);}
	if(strcmp(s.c,"cl") == 0)   {return lValNativeFunc(lnfCl);}
	if(strcmp(s.c,"mem") == 0)  {return lValNativeFunc(lnfMem);}
	return lValNil();
}

static lVal *lDefineClosureSym(lClosure *c,const lSymbol s){
	if(c == NULL){return NULL;}
	lVal *v;
	for(v = c->data;v->next != NULL;v = v->next){
		if(v->type != ltList)         {continue;}
		if(v->vList == NULL)          {continue;}
		if(v->vList->type != ltSymbol){continue;}
		if(v->vList->vSymbol.v != s.v){continue;}
		return NULL;
	}
	if(v == NULL){return NULL;}
	v->next = lValNil();
	v->type = ltList;
	v->vList = lValAlloc();
	if(v->vList == NULL){
		v->vList = NULL;
		return NULL;
	}
	v->vList->type    = ltSymbol;
	v->vList->vSymbol = s;
	v->vList->next    = lValNil();
	if(v->vList->next == NULL){
		v->vList = NULL;
		return NULL;
	}
	return v->vList->next;
}

static lVal *lResolveClosureSymbol(lClosure *c, const lSymbol s){
	if(c == NULL){return NULL;}
	for(lVal *v = c->data;v != NULL;v = v->next){
		if(v->type != ltList)         {continue;}
		if(v->vList == NULL)          {continue;}
		if(v->vList->type != ltSymbol){continue;}
		if(v->vList->vSymbol.v != s.v){continue;}
		return v->vList->next;
	}
	return lResolveClosureSymbol(c->parent,s);
}

lVal *lResolveSym(lClosure *c, const lSymbol s){
	lVal *v = lResolveClosureSymbol(c,s);
	if(v != NULL){
		return v;
	}else{
		return lResolveNativeSym(s);
	}
}

lVal *lEval(lClosure *c, lVal *v){
	lVal *ret = v;
	if(v == NULL){return NULL;}
	if(v->type == ltSymbol){
		ret = lResolveSym(c,v->vSymbol);
	} else if(v->type == ltLambda){
		ret = lEval(c,v->vList);
		if((ret != NULL) && (ret->type == ltNativeFunc)){
			ret = ret->vNativeFunc(c,v->vList->next);
		}
	}
	return ret;
}

int lMemUsage(){
	int used=0;
	for(uint i=0;i<lValMax;i++){
		if(lValList[i].type == ltNoAlloc){continue;}
		used++;
	}
	return used;
}

static inline void lValGCMark(lVal *sv){
	for(lVal *v = sv;v != NULL;v = v->next){
		v->flags |= lfMarked;
		switch(v->type){
		default:
			break;
		case ltList:
			lValGCMark(v->vList);
			break;
		case ltLambda:
			lValGCMark(v->vLambda);
			break;
		case ltClosure:
			lValGCMark(v->vClosure->data);
			break;
		}
	}
}
void lClosureGC(lClosure *c){
	for(uint i=0;i<lValMax;i++){lValList[i].flags &= ~lfMarked;}
	for(;c != NULL;c = c->parent){lValGCMark(c->data);}
	for(uint i=0;i<lValMax;i++){
		if(lValList[i].type == ltNoAlloc){continue;}
		if(lValList[i].flags & lfMarked) {continue;}
		lValFree(&lValList[i]);
	}
}
