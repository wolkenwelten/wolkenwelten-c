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
			t = snprintf(cur,bufEnd-cur,"(λ (cl ");
			if(t > 0){cur += t;}
			for(lVal *n = v->vLambda->data;n != NULL;n = n->next){
				cur = lSPrintVal(n,cur,bufEnd);
				if(n->next != NULL){*cur++ = ' ';}
			}
			t = snprintf(cur,bufEnd-cur," ) ");
			if(t > 0){cur += t;}
			for(lVal *n = v->vLambda->text;n != NULL;n = n->next){
				cur = lSPrintVal(n,cur,bufEnd);
				if(n->next != NULL){*cur++ = ' ';}
			}
			t = snprintf(cur,bufEnd-cur,")");
			break; }
		case ltList: {
			t = snprintf(buf,bufEnd-cur,"(");
			if(t > 0){cur += t;}
			for(lVal *n = v->vList;n != NULL;n = n->next){
				cur = lSPrintVal(n,cur,bufEnd);
				if(n->type == ltNoAlloc){
					*cur++ = '.';
					*cur++ = '.';
					*cur++ = '.';
					*cur++ = ' ';
					break;
				}
				if(n->next != NULL){*cur++ = ' ';}
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

char *lSPrintChain(lVal *start, char *buf, char *bufEnd){
	if(start == NULL){return buf;}
	char *cur = buf;
	for(lVal *v = start;v!=NULL;v = v->next){
		cur = lSPrintVal(v,cur,bufEnd);
		if((bufEnd - cur) < 1){return cur;}
		*cur++ = ' ';
		if(v->type == ltNoAlloc){break;}
	}
	*bufEnd = 0;
	return cur;
}

lVal *lnfLen(lClosure *c, lVal *v){
	if(v == NULL){return lValNil();}
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
	lVal *str = lEval(c,v);
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
	if(v->next != NULL){
		lVal *lStart = lEval(c,v->next);
		if(lStart->type == ltInt){
			start = lStart->vInt;
		}
		if(v->next->next != NULL){
			lVal *lLen = lEval(c,v->next->next);
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
	for(lVal *sexpr = v; sexpr != NULL; sexpr = sexpr->next){
		lVal *t = lEval(c,sexpr);
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
