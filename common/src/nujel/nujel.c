#include "nujel.h"

#include "arithmetic.h"
#include "boolean.h"
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
	c->data = lValAlloc();
	if(c->data == NULL){return NULL;}
	c->data->type = ltNil;
	return c;
}

static void lStringAdvanceToNextCharacter(lCString *s){
	for(;(*s->data != 0) && (isspace(*s->data));s->data++){}
}

static lVal *lParseString(lCString *s){
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
			lString *vs = v->vString = lStringAlloc();
			vs->data = vs->buf = malloc((b-buf)+1);
			memcpy(vs->buf,buf,b-buf);
			vs->len = b-buf;
			vs->data[vs->len] = 0;
			vs->bufEnd = vs->data + vs->len;
			return v;
		}else{
			*b++ = *s->data++;
		}
	}
	lVal *v = lValAlloc();
	v->type = ltNil;
	return v;
}

static lVal *lParseNumber(lCString *s){
	lVal *v = lValInt(0);
	int c   = *s->data;
	int fc  = c;
	bool isFloat = false;
	int cval = 0;
	int digits = 0;
	if(fc == '-'){c = *++s->data;}
	while(!isspace(c)){
		if(c == 0){break;}
		if(isdigit(c)){
			cval *= 10;
			cval += c - '0';
			digits++;
		}else if(c == '.'){
			isFloat = true;
			v->vInt = cval;
			cval    = 0;
			digits  = 0;
		}else if((c != ',') && (c != '_')){
			break;
		}
		c = *++s->data;
	}
	if(isFloat){
		int t     = v->vInt;
		v->type   = ltFloat;
		float tv  = cval / powf(10.f,digits);
		v->vFloat = t + tv;
	}else{
		v->vInt   = cval;
	}
	if(fc == '-'){
		if(isFloat){
			v->vFloat = -v->vFloat;
		}else{
			v->vInt = -v->vInt;
		}
	}
	return v;
}

static lVal *lParseSymbol(lCString *s){
	lVal *v = lValAlloc();
	v->type = ltSymbol;
	v->vSymbol.v[0] = 0;
	v->vSymbol.v[1] = 0;
	for(int i=0;i<16;i++){
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

static lVal *lParseSpecial(lCString *s){
	lVal *v = lValNil();
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

static lVal *lParse(lCString *s){
	lVal *v = NULL, *ret = NULL;
	while(1){
		lStringAdvanceToNextCharacter(s);
		char c = *s->data;
		lVal *t = NULL;
		if((c == 0) || (c == ')') || (s->data >= s->bufEnd)){
			s->data++;
			return ret;
		}else if(c == '('){
			s->data+=1;
			t = lParse(s);
		}else if((c == '\'') && (s->data[1] == '(')){
			s->data += 2;
			t = lCons(lValSym("quote"),lParse(s));
		}else if(c == '"'){
			s->data++;
			t = lParseString(s);
		}else if(isdigit(c)){
			t = lParseNumber(s);
		}else if(c == '#'){
			t = lParseSpecial(s);
		}else if((c == '-') && (isdigit(s->data[1]))){
			t = lParseNumber(s);
		}else{
			t = lParseSymbol(s);
		}
		t = lCons(t,NULL);
		if(v != NULL){v->vList.cdr = t;}
		v = t;
	}
}

lVal *lParseSExprCS(const char *str){
	lCString *s = lCStringAlloc();
	s->data     = str;
	int len     = strlen(str);
	s->bufEnd   = &str[len];
	lVal *ret   = lParse(s);
	lCStringFree(s);
	return ret;
}

void lPrintChain(lVal *v){
	static char buf[8192];
	char *end = lSPrintChain(v,buf,&buf[sizeof(buf) - 1]);
	*end = 0;
	printf("%s\n",buf);
}

void lPrintVal(lVal *v){
	static char buf[256];
	lSPrintVal(v,buf,&buf[sizeof(buf)]);
	printf("%s\n",buf);
}

static lVal *lnfDefine(lClosure *c, lClosure *ec, lVal *v){
	if(v == NULL)                {return lValNil();}
	if(v->type == ltSymbol){
		return lDefineClosureSym(c,v->vSymbol);
	}else if(v->type == ltList){
		lVal *sym = v->vList.car;
		if(sym->type != ltSymbol)    {sym = lEval(c,sym);}
		if(sym->type != ltSymbol)    {return lValNil();}
		lVal *t = lDefineClosureSym(c,sym->vSymbol);
		if(t == NULL)                {return lValNil();}
		lVal *val = lEval(ec,v->vList.cdr);
		if(val != NULL){lValCopy(t,val);}
		return t;
	}
	return lValNil();
}

static lVal *lnfDef(lClosure *c, lVal *v){
	return lnfDefine(c,c,v);
}

static lVal *lnfSet(lClosure *c, lVal *v){
	if(v == NULL)                {return lValNil();}
	if(v->type != ltList)        {return lValNil();}
	lVal *sym = v->vList.car;
	if(sym->type != ltSymbol)    {sym = lEval(c,sym);}
	if(sym->type != ltSymbol)    {return lValNil();}
	lVal *t = lResolveClosureSym(c,sym->vSymbol);
	if(t == NULL)                {return lValNil();}
	lVal *val = lEval(c,v->vList.cdr);
	if(val == NULL){return t;}
	lValCopy(t,val);
	return t;
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

static lVal *lnfLambda(lClosure *c, lVal *v){
	lClosure *cl = lClosureNew(c);
	if((cl == NULL) || (v == NULL) || (v->vList.car == NULL) || (v->vList.cdr == NULL)){return NULL;}
	cl->text = v->vList.cdr;
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vLambda = cl;

	foreach(n,v->vList.car){
		if(n->vList.car->type != ltSymbol){continue;}
		lVal *t = lDefineClosureSym(cl,n->vSymbol);
		(void)t;
	}

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
	if(v == NULL)          {return lValNil();}
	if((v->type != ltList)){return lValNil();}
	lClosure *nc = lClosureNew(c);
	foreach(n,v->vList.car){
		lnfDefine(c,nc,v);
	}
	lVal *ret = NULL;
	foreach(n,v->vList.cdr){
		ret = lEval(nc,n);
	}
	if(ret == NULL){ret = lValNil();}
	return ret;
}

static lVal *lLambda(lClosure *c,lVal *v, lClosure *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	lVal *vn = v;
	lClosure *tmpc = lClosureNew(lambda);
	foreach(n,lambda->data){
		lVal *nn = n->vList.car;
		if(nn->type != ltList)             {continue;}
		if(nn->vList.car != NULL)          {continue;}
		if(nn->vList.car->type != ltSymbol){continue;}
		lVal *lv = lDefineClosureSym(tmpc,nn->vList.car->vSymbol);
		lVal *t  = lEval(c,vn);
		if(t  != NULL){*lv = *t;}
		if(vn != NULL){vn = vn->vList.cdr;}
	}

	lVal *ret = NULL;
	foreach(n,lambda->text){
		ret = lEval(tmpc,n);
	}
	if(ret == NULL){ret = lValNil();}
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
	foreach(n,v){
		if(n->type != ltList)                 {continue;}
		if(n->vList.car == NULL)              {continue;}
		if(n->vList.car->type != ltList)      {continue;}
		lVal *t = lEval(c,n->vList.car->vList.car);
		if(t == NULL)                         {continue;}
		if(t->type == ltNil)                  {continue;}
		if((t->type == ltBool) && (!t->vBool)){continue;}
		return lEval(c,n->vList.car->vList.cdr);
	}
	return lValNil();
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL)         {return lValNil();}
	if(v->type != ltList) {return lValNil();}
	lVal *pred = lEval(c,v->vList.car);
	if(pred == NULL)      {return lValNil();}
	v = v->vList.cdr;
	if(v == NULL)         {return lValNil();}

	if((pred->type == ltNil) || ((pred->type == ltBool) && (pred->vBool == false))){
		v = v->vList.cdr;
	}

	return lEval(c,v->vList.car);
}

static lVal *lnfWhen(lClosure *c, lVal *v){
	if(v == NULL)         {return lValNil();}
	if(v->type != ltList) {return lValNil();}
	lVal *pred = lEval(c,v->vList.car);
	if(pred == NULL)      {return lValNil();}
	if((pred->type == ltNil) || ((pred->type == ltBool) && (pred->vBool == false))){
		return lValNil();
	}
	return lEval(c,v->vList.cdr);
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

	if(strcmp(s.c,"cl") == 0)     {return lValNativeFunc(lnfCl);}
	if(strcmp(s.c,"mem") == 0)    {return lValNativeFunc(lnfMem);}

	if(strcmp(s.c,"if") == 0)     {return lValNativeFunc(lnfIf);}
	if(strcmp(s.c,"when") == 0)   {return lValNativeFunc(lnfWhen);}
	if(strcmp(s.c,"cond") == 0)   {return lValNativeFunc(lnfCond);}

	if(strcmp(s.c,"define") == 0) {return lValNativeFunc(lnfDef);}
	if(strcmp(s.c,"let") == 0)    {return lValNativeFunc(lnfLet);}
	if(strcmp(s.c,"quote") == 0)  {return lValNativeFunc(lnfQuote);}
	if(strcmp(s.c,"set!") == 0)   {return lValNativeFunc(lnfSet);}

	if(strcmp(s.c,"λ") == 0)      {return lValNativeFunc(lnfLambda);}
	if(strcmp(s.c,"lambda") == 0) {return lValNativeFunc(lnfLambda);}

	if(strcmp(s.c,"add") == 0)    {return lValNativeFunc(lnfAdd);}
	if(strcmp(s.c,"div") == 0)    {return lValNativeFunc(lnfDiv);}
	if(strcmp(s.c,"sub") == 0)    {return lValNativeFunc(lnfSub);}
	if(strcmp(s.c,"mul") == 0)    {return lValNativeFunc(lnfMul);}
	if(strcmp(s.c,"mod") == 0)    {return lValNativeFunc(lnfMod);}
	if(strcmp(s.c,"modulo") == 0) {return lValNativeFunc(lnfMod);}
	if(strcmp(s.c,"int") == 0)    {return lValNativeFunc(lnfInt);}
	if(strcmp(s.c,"float") == 0)  {return lValNativeFunc(lnfFloat);}
	if(strcmp(s.c,"vec") == 0)    {return lValNativeFunc(lnfVec);}
	if(strcmp(s.c,"abs") == 0)    {return lValNativeFunc(lnfAbs);}
	if(strcmp(s.c,"vx") == 0)     {return lValNativeFunc(lnfVX);}
	if(strcmp(s.c,"vy") == 0)     {return lValNativeFunc(lnfVY);}
	if(strcmp(s.c,"vz") == 0)     {return lValNativeFunc(lnfVZ);}

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

lVal *lDefineClosureSym(lClosure *c,const lSymbol s){
	if(c == NULL){return NULL;}
	lVal *cdr = NULL;
	foreach(v,c->data){
		cdr = v;
		const lVal *n = v->vList.car;
		if(n == NULL)                  {continue;}
		if(n->type != ltList)          {continue;}
		const lVal *sym = n->vList.car;
		if(sym->type != ltSymbol)      {continue;}
		if(sym->vSymbol.v[0] != s.v[0]){continue;}
		if(sym->vSymbol.v[1] != s.v[1]){continue;}
		return NULL;
	}
	if(cdr == NULL){return NULL;}
	lVal *t = lCons(lValSymS(s),lValNil());
	if(t == NULL){return NULL;}
	cdr->vList.cdr = lCons(t,NULL);
	return t->vList.cdr;
}

lVal *lResolveClosureSym(lClosure *c, const lSymbol s){
	if(c == NULL){return NULL;}
	foreach(v,c->data){
		const lVal *n = v->vList.car;
		if(n == NULL)                  {continue;}
		if(n->type != ltList)          {continue;}
		const lVal *sym = n->vList.car;
		if(sym->type != ltSymbol)      {continue;}
		if(sym->vSymbol.v[0] != s.v[0]){continue;}
		if(sym->vSymbol.v[1] != s.v[1]){continue;}
		return n->vList.cdr;
	}
	return lResolveClosureSym(c->parent,s);
}

lVal *lResolveSym(lClosure *c, const lSymbol s){
	lVal *v = lResolveClosureSym(c,s);
	if(v != NULL){
		return v;
	}else{
		return lResolveNativeSym(s);
	}
}

lVal *lEval(lClosure *c, lVal *v){
	lVal *ret = v;
	if(c == NULL){return NULL;}
	if(v == NULL){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c,v->vSymbol);
	} else if(v->type == ltList){
		ret = lEval(c,v->vList.car);

		if(ret != NULL){
			if(ret->type == ltNativeFunc){
				ret = ret->vNativeFunc(c,v->vList.cdr);
			}else if(ret->type == ltLambda){
				ret = lLambda(c,v->vList.cdr,ret->vLambda);
			}
		}

		if(v->vList.cdr != NULL){
			return lEval(c,v->vList.cdr);
		}
		return ret;
	}else{
		return v;
	}
}

int lMemUsage(){
	int used=0;
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		used++;
	}
	return used;
}

static void lValGCMark(lVal *sv){
	foreach(v,sv){
		v->flags |= lfMarked;
		lVal *car = v->vList.car;
		if(car == NULL){continue;}
		if(car->type == ltList)   {lValGCMark(car);}
		if(car->type == ltLambda) {
			lValGCMark(car->vLambda->data);
			lValGCMark(car->vLambda->text);
		}
	}
}
void lClosureGC(){
	for(uint i=0;i<lValMax;i++){lValBuf[i].flags &= ~lfMarked;}
	//for(;c != NULL;c = c->parent){lValGCMark(c->data);}
	for(uint i=0;i<lClosureMax;i++){lValGCMark(lClosureList[i].data);}
	for(uint i=0;i<lClosureMax;i++){lValGCMark(lClosureList[i].text);}
	for(uint i=0;i<lValMax;i++){
		if(lValBuf[i].type == ltNoAlloc){continue;}
		if(lValBuf[i].flags & lfMarked) {continue;}
		lValFree(&lValBuf[i]);
	}
}

lType lTypecast(lVal *a, lVal *b){
	if((a->type == ltFloat) || (b->type == ltFloat)){return ltFloat;}
	if((a->type == ltInt)   || (b->type == ltInt)  ){return ltInt;}
	if((a->type == ltBool)  || (b->type == ltBool) ){return ltBool;}
	if(a->type == b->type){ return a->type;}
	return ltNil;
}
