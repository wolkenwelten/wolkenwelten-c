#include "string.h"

#include "nujel.h"

#include <stdio.h>
#include <string.h>

char *ansiRS = "\033[0m";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

int bufPrintFloat(float v, char *buf, int t, int len){
	t = snprintf(buf,len,"%.5f",v);
	for(;buf[t-1] == '0';t--){buf[t]=0;}
	if(buf[t] == '0'){buf[t] = 0;}
	if(buf[t-1] == '.'){buf[t++] = '0';}
	return t;
}

char *lSPrintVal(lVal *v, char *buf, char *bufEnd){
	if(v == NULL){return buf;}
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
		case ltLambda: {
			t = snprintf(cur,bufEnd-cur,"(λ ");

			if(t > 0){cur += t;}
			forEach(n,v->vLambda->data){
				cur = lSPrintVal(n,cur,bufEnd);
				if(n->vList.cdr != NULL){*cur++ = ' ';}
			}
			*cur++ = ' ';
			forEach(n,v->vLambda->text){
				cur = lSPrintVal(n->vList.car,cur,bufEnd);
				if(n->vList.cdr != NULL){*cur++ = ' ';}
			}
			t = snprintf(cur,bufEnd-cur,")");
			break; }
		case ltList: {
			if(v->vList.car->type == ltSymbol){
				if((v->vList.car->vSymbol.v[0] == symQuote.v[0]) &&
				   (v->vList.car->vSymbol.v[1] == symQuote.v[1]) &&
				   (v->vList.cdr != NULL)){
					v = v->vList.cdr->vList.car;
					*cur++ = '\'';
				}
			}
			t = snprintf(cur,bufEnd-cur,"(");
			if(t > 0){cur += t;}
			forEach(n,v){
				cur = lSPrintVal(n->vList.car,cur,bufEnd);
				if(n->type == ltNoAlloc){
					*cur++ = '.';
					*cur++ = '.';
					*cur++ = '.';
					*cur++ = ' ';
					break;
				}
				if(n->vList.cdr != NULL){*cur++ = ' ';}
			}
			t = snprintf(cur,bufEnd-cur,")");
			break; }
		case ltInt:
			t = snprintf(buf,len,"%i",v->vInt);
			break;
		case ltFloat:
			t = bufPrintFloat(v->vFloat,buf,t,len);
			break;
		case ltVec:
			t  = snprintf(buf,len,"(vec ");
			t += bufPrintFloat(v->vVec.x,&buf[t],t,len);
			buf[t++] = ' ';
			t += bufPrintFloat(v->vVec.y,&buf[t],t,len);
			buf[t++] = ' ';
			t += bufPrintFloat(v->vVec.z,&buf[t],t,len);
			t += snprintf(&buf[t],len,")");
			break;
		case ltCString:
		case ltString:
			t = snprintf(buf,len,"\"%s\"",v->vString->data);
			break;
		case ltSymbol:
			t = snprintf(buf,len,"%.16s",v->vSymbol.c);
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

lVal *lnfLen(lClosure *c, lVal *v){
	if(v == NULL){return lValNil();}
	if(v->type == ltList){v = v->vList.car;}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValNil();}
	switch(t->type){
	default: return lValNil();
	case ltCString:
		if(t->vCString == NULL){return lValInt(0);}
		return lValInt(t->vCString->bufEnd - t->vCString->data);
	case ltString:
		if(t->vString == NULL){return lValInt(0);}
		return lValInt(t->vString->len);
	}
}

lVal *lnfSubstr(lClosure *c, lVal *v){
	const char *buf;
	int start = 0;
	int len   = 0;
	int slen  = 0;
	if(v == NULL){return lValNil();}
	lVal *str = lEval(c,v->vList.car);
	if(str == NULL){return lValNil();}
	switch(str->type){
	default: return lValNil();
	case ltCString:
		if(str->vCString == NULL){return lValNil();}
		buf  = str->vCString->data;
		slen = len = str->vCString->bufEnd - str->vCString->data;
		break;
	case ltString:
		if(str->vString == NULL){return lValNil();}
		buf  = str->vCString->data;
		slen = len = str->vString->len;
		break;
	}
	if(v->vList.cdr != NULL){
		v = v->vList.cdr;
		lVal *lStart = lEval(c,v->vList.car);
		if(lStart->type == ltInt){
			start = lStart->vInt;
		}
		if(v->vList.cdr != NULL){
			v = v->vList.cdr;
			lVal *lLen = lEval(c,v->vList.car);
			if(lLen->type == ltInt){
				len = lLen->vInt;
			}
		}
	}
	if(start >= slen){return lValNil();}
	if(start < 0){start = slen + start;}
	if(len < 0)  {len   = slen + len;}
	len = MIN(slen,len-start);

	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltString;
	ret->vString = lStringNew(&buf[start], len);
	return ret;
}

lVal *lnfBr(lClosure *c, lVal *v){
	char tmpStringBuf[256];
	int nr = 1;
	if(v != NULL){
		lVal *t = lEval(c,v);
		if((t != NULL) && (t->type == ltInt)){
			nr = t->vInt;
		}
	}
	char *buf = tmpStringBuf;
	for(;nr>0;nr--){
		*buf++='\n';
	}
	*buf++ = 0;
	return lValString(tmpStringBuf);
}

lVal *lnfCat(lClosure *c, lVal *v){
	char tmpStringBuf[512];
	char *buf = tmpStringBuf;
	int len = 0;
	forEach(sexpr,v){
		lVal *t = lEval(c,sexpr->vList.car);
		if(t == NULL){continue;}
		switch(t->type){
		default: break;
		case ltInt: {
			int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%i",t->vInt);
			len += clen;
			buf += clen;
			break; }
		case ltCString: {
			if(t->vCString == NULL){continue;}
			int clen = t->vCString->bufEnd - t->vCString->data;
			memcpy(buf,t->vCString->data,clen);
			len += clen;
			buf += clen;
			break; }
		case ltString:
			if(t->vString == NULL){continue;}
			memcpy(buf,t->vString->data,t->vString->len);
			len += t->vString->len;
			buf += t->vString->len;
			break;
		}
	}

	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vString = lStringNew(tmpStringBuf, len);
	return ret;
}

lVal *lnfString(lClosure *c, lVal *t){
	char tmpStringBuf[512];
	char *buf = tmpStringBuf;
	int len = 0;
	if(t == NULL){return lValString("");}
	(void)c;

	switch(t->type){
	default: break;
	case ltFloat: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%f",t->vFloat);
		len += clen;
		buf += clen;
		break; }
	case ltInt: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%i",t->vInt);
		len += clen;
		buf += clen;
		break; }
	case ltCString: {
		if(t->vCString == NULL){return lValString("");}
		int clen = t->vCString->bufEnd - t->vCString->data;
		memcpy(buf,t->vCString->data,clen);
		len += clen;
		buf += clen;
		break; }
	case ltString:
		if(t->vString == NULL){return lValString("");}
		memcpy(buf,t->vString->data,t->vString->len);
		len += t->vString->len;
		buf += t->vString->len;
		break;
	}

	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vString = lStringNew(tmpStringBuf, len);
	return ret;
}

lVal *lnfAnsiFG(lClosure *c, lVal *v){
	int i = 0;
	if(v != NULL){
		lVal *t = lEval(c,v);
		if((t != NULL) && (t->type == ltInt)){
			i = t->vInt;
		}
	}
	if(i < 0){i = 0;}
	if(i > 16){i = 15;}
	return lValString(ansiFG[i]);
}

lVal *lnfAnsiRS(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValString(ansiRS);
}
