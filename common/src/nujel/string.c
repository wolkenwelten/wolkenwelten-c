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
#include "string.h"

#include "nujel.h"
#include "casting.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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

int bufWriteString(char *buf, int len, const char *data){
	u8 c;
	int written=0;
	buf[written++] = '\"';
	while((c = *data++) != 0){
		if((written + 2) > len){break;}
		if(c == '\n'){
			buf[written++] = '\\';
			buf[written++] = 'n';
			continue;
		}else if(c == '"'){
			buf[written++] = '\\';
			buf[written++] = '"';
			continue;
		}else if(c == '\''){
			buf[written++] = '\\';
			buf[written++] = '\'';
			continue;
		}else if(c == '\\'){
			buf[written++] = '\\';
			buf[written++] = '\\';
			continue;
		}
		buf[written++] = c;
	}
	buf[written++] = '\"';
	return written;
}

char *lSWriteVal(lVal *v, char *buf, char *bufEnd){
	*buf = 0;
	if(v == NULL){return buf;}
	char *cur = buf;
	int t = 0;
	int len = bufEnd-buf;

	switch(v->type){
	case ltNoAlloc:
		t = snprintf(buf,len,"#zzz");
		break;
	case ltBool:
		if(v->vBool){
			t = snprintf(buf,len,"#t");
		}else{
			t = snprintf(buf,len,"#f");
		}
		break;
	case ltLambda: {
		lClosure *cl = &lClosureList[v->vCdr & CLO_MASK];
		if(cl->flags & lfObject){
			t = snprintf(cur,bufEnd-cur,"(ω (");
		}else if(cl->flags & lfDynamic){
			t = snprintf(cur,bufEnd-cur,"(δ (");
		}else{
			t = snprintf(cur,bufEnd-cur,"(λ (");
		}
		if(t > 0){cur += t;}
		forEach(n,lCloData(v->vCdr)){
			if(n->vList.car == NULL)            {continue;}
			if(n->vList.car->type != ltPair)    {continue;}
			if(n->vList.car->vList.car == NULL) {continue;}
			if((n->vList.car->vList.cdr != NULL) && (n->vList.car->vList.cdr->type == ltPair) && (n->vList.car->vList.cdr->vList.car != NULL)){
				cur = lSWriteVal(n->vList.car,cur,bufEnd);
			}else{
				cur = lSWriteVal(n->vList.car->vList.car,cur,bufEnd);
			}
			if(n->vList.cdr != NULL){*cur++ = ' ';}
		}
		*cur++ = ')';
		*cur++ = ' ';
		forEach(n,lCloText(v->vCdr)){
			cur = lSWriteVal(n->vList.car,cur,bufEnd);
			if(n->vList.cdr != NULL){*cur++ = ' ';}
		}
		t = snprintf(cur,bufEnd-cur,")");
		break; }
	case ltPair: {
		if((v->vList.car != NULL) && (v->vList.car->type == ltSymbol)){
			if((v->vList.car->vSymbol.v[0] == symQuote.v[0]) &&
			   (v->vList.car->vSymbol.v[1] == symQuote.v[1]) &&
			   (v->vList.cdr != NULL)){
				v = v->vList.cdr->vList.car;
				*cur++ = '\'';
			}
		}
		t = snprintf(cur,bufEnd-cur,"(");
		if(t > 0){cur += t;}
		for(lVal *n = v;n != NULL; n = n->vList.cdr){
			if(n->type == ltPair){
				cur = lSWriteVal(n->vList.car,cur,bufEnd);
				if(n->vList.cdr != NULL){*cur++ = ' ';}
			}else{
				*cur++ = '.';
				*cur++ = ' ';
				cur = lSWriteVal(n,cur,bufEnd);
				break;
			}
		}
		t = snprintf(cur,bufEnd-cur,")");
		break; }
	case ltArray: {
		t = snprintf(cur,bufEnd-cur,"#(");
		if(t > 0){cur += t;}
		if(lArrData(v) != NULL){
			const int arrLen = lArrLength(v);
			for(int i=0;i<arrLen;i++){
				cur = lSWriteVal(lArrData(v)[i],cur,bufEnd);
				if(i < (lArrLength(v)-1)){*cur++ = ' ';}
			}
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
	case ltString:
		t = bufWriteString(buf,len,lStrData(v));
		break;
	case ltSymbol:
		t = snprintf(buf,len,"%.16s",v->vSymbol.c);
		break;
	case ltNativeFunc:
		t = snprintf(buf,len,"#cfn_%u",v->vCdr);
		break;
	case ltInf:
		t = snprintf(buf,len,"#inf");
		break;
	}

	if(t > 0){cur += t;}
	*cur = 0;
	return cur;
}

char *lSDisplayVal(lVal *v, char *buf, char *bufEnd){
	if(v == NULL){return buf;}
	char *cur = buf;
	int t = 0;
	const int len = bufEnd-buf;

	switch(v->type){
	case ltNoAlloc:
		t = snprintf(buf,len,"#zzz");
		break;
	case ltBool:
		if(v->vBool){
			t = snprintf(buf,len,"#t");
		}else{
			t = snprintf(buf,len,"#f");
		}
		break;
	case ltLambda: {
		lClosure *cl = &lClosureList[v->vCdr & CLO_MASK];
		if(cl->flags & lfObject){
			t = snprintf(cur,bufEnd-cur,"(ω (");
		}else if(cl->flags & lfDynamic){
			t = snprintf(cur,bufEnd-cur,"(δ (");
		}else{
			t = snprintf(cur,bufEnd-cur,"(λ (");
		}

		if(t > 0){cur += t;}
		forEach(n,lCloData(v->vCdr)){
			if(n->vList.car == NULL)           {continue;}
			if(n->vList.car->type != ltPair)   {continue;}
			if(n->vList.car->vList.car == NULL){continue;}
			if((n->vList.car->vList.cdr != NULL) && (n->vList.car->vList.cdr->type == ltPair) && (n->vList.car->vList.cdr->vList.car != NULL)){
				cur = lSWriteVal(n->vList.car,cur,bufEnd);
			}else{
				cur = lSWriteVal(n->vList.car->vList.car,cur,bufEnd);
			}
			if(n->vList.cdr != NULL){*cur++ = ' ';}
		}
		*cur++ = ')';
		*cur++ = ' ';
		forEach(n,lCloText(v->vCdr)){
			cur = lSWriteVal(n->vList.car,cur,bufEnd);
			if(n->vList.cdr != NULL){*cur++ = ' ';}
		}
		t = snprintf(cur,bufEnd-cur,")");
		break; }
	case ltPair: {
		if((v->vList.car != NULL) && (v->vList.car->type == ltSymbol)){
			if((v->vList.car->vSymbol.v[0] == symQuote.v[0]) &&
			   (v->vList.car->vSymbol.v[1] == symQuote.v[1]) &&
			   (v->vList.cdr != NULL)){
				v = v->vList.cdr->vList.car;
				*cur++ = '\'';
			}
		}
		t = snprintf(cur,bufEnd-cur,"(");
		if(t > 0){cur += t;}
		for(lVal *n = v;n != NULL; n = n->vList.cdr){
			if(n->type == ltPair){
				cur = lSWriteVal(n->vList.car,cur,bufEnd);
				if(n->vList.cdr != NULL){*cur++ = ' ';}
			}else{
				*cur++ = '.';
				*cur++ = ' ';
				cur = lSWriteVal(n,cur,bufEnd);
				break;
			}
		}
		t = snprintf(cur,bufEnd-cur,")");
		break; }
	case ltArray: {
		t = snprintf(cur,bufEnd-cur,"#(");
		if(t > 0){cur += t;}
		if(lArrData(v) != NULL){
			for(int i=0;i<lArrLength(v);i++){
				cur = lSDisplayVal(lArrData(v)[i],cur,bufEnd);
				if(i < (lArrLength(v)-1)){*cur++ = ' ';}
			}
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
	case ltString:
		t = snprintf(buf,len,"%s",lStrData(v));
		break;
	case ltSymbol:
		t = snprintf(buf,len,"%.16s",v->vSymbol.c);
		break;
	case ltNativeFunc:
		t = snprintf(buf,len,"#cfn_%u",v->vCdr);
		break;
	case ltInf:
		t = snprintf(buf,len,"#inf");
		break;
	}

	if(t > 0){cur += t;}
	*cur = 0;
	return cur;
}

lVal *lnfStrlen(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	return lValInt(lStringLength(&lStr(t)));
}

lVal *lnfStrDown(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const int len = lStringLength(&lStr(t));
	char *buf = malloc(len+1);
	for(int i=0;i<len;i++){
		buf[i] = tolower((u8)lStrData(t)[i]);
	}
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStr(ret).flags |= lfHeapAlloc;
	lStr(ret).buf = lStr(ret).data = buf;
	lStr(ret).bufEnd = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfStrUp(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const int len = lStringLength(&lStr(t));
	char *buf = malloc(len+1);
	for(int i=0;i<len;i++){
		buf[i] = toupper((u8)lStrData(t)[i]);
	}
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStrFlags(ret) |= lfHeapAlloc;
	lStrBuf(ret) = lStrData(ret) = buf;
	lStrEnd(ret) = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfStrCap(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const int len = lStringLength(&lStr(t));
	char *buf = malloc(len+1);
	int cap = 1;
	for(int i=0;i<len;i++){
		if(isspace((u8)lStrData(t)[i])){
			cap = 1;
			buf[i] = lStrData(t)[i];
		}else{
			if(cap){
				buf[i] = toupper((u8)lStrData(t)[i]);
				cap = 0;
			}else{
				buf[i] = tolower((u8)lStrData(t)[i]);
			}
		}
	}
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStrFlags(ret) |= lfHeapAlloc;
	lStrBuf(ret) = lStrData(ret) = buf;
	lStrEnd(ret) = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfSubstr(lClosure *c, lVal *v){
	const char *buf;
	int start = 0;
	int len   = 0;
	int slen  = 0;
	if(v == NULL){return NULL;}
	lVal *str = lEval(c,v->vList.car);
	if(str == NULL){return NULL;}
	switch(str->type){
	default: return NULL;
	case ltString:
		if(str->vCdr == 0){return NULL;}
		buf  = lStrData(str);
		slen = len = lStringLength(&lStr(str));
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
	if(start >= slen){return NULL;}
	if(start < 0){start = slen + start;}
	if(len < 0)  {len   = slen + len;}
	len = MIN(slen,len-start);

	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltString;
	ret->vCdr = lStringNew(&buf[start], len);
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
	char tmpStringBuf[8192];
	char *buf = tmpStringBuf;
	int len = 0;
	forEach(sexpr,v){
		lVal *t = lEval(c,sexpr->vList.car);
		int clen = 0;
		if(t == NULL){continue;}
		//lWriteVal(t);
		switch(t->type){
		default: break;
		case ltSymbol: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",t->vSymbol.c);
			break; }
		case ltFloat: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%.5f",t->vFloat);
			for(;buf[clen-1] == '0';clen--){buf[clen]=0;}
			if(buf[clen] == '0'){buf[clen] = 0;}
			if(buf[clen-1] == '.'){buf[clen++] = '0';}
			break; }
		case ltInt: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%i",t->vInt);
			break; }
		case ltString:
			if(t->vCdr == 0){continue;}
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",lStrData(t));
			break;
		}
		if(clen > 0){
			len += clen;
			buf += clen;
		}
	}
	*buf = 0;
	return lValString(tmpStringBuf);
}

lVal *lnfAnsiFG(lClosure *c, lVal *v){
	int i = 0;
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,lCarOrV(v)));
		if((t != NULL) && (t->type == ltInt)){
			i = t->vInt;
		}
	}
	if(i < 0) {i =  0;}
	if(i > 16){i = 15;}
	return lValString(ansiFG[i]);
}

lVal *lnfAnsiRS(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValString(ansiRS);
}

lVal *lnfStrSym(lClosure *c, lVal *v){
	v = lEval(c,lCarOrV(v));
	if(v == NULL){return NULL;}
	if(v->type != ltString){return NULL;}
	return lValSym(lStrData(v));
}

lVal *lnfSymStr(lClosure *c, lVal *v){
	v = lEval(c,lCarOrV(v));
	if(v == NULL){return NULL;}
	if(v->type != ltSymbol){return NULL;}
	return lValString(v->vSymbol.c);
}

void lAddStringFuncs(lClosure *c){
	lAddNativeFunc(c,"cat",           "(...args)",       "ConCATenates ...args into a single string",                                               lnfCat);
	lAddNativeFunc(c,"str-len",       "(s)",             "Returns length of string s",                                                              lnfStrlen);
	lAddNativeFunc(c,"str-up",        "(s)",             "Returns a copy of string s all uppercased",                                               lnfStrUp);
	lAddNativeFunc(c,"str-down",      "(s)",             "Returns a copy of string s all lowercased",                                               lnfStrDown);
	lAddNativeFunc(c,"str-capitalize","(s)",             "Returns a copy of string s capitalized",                                                  lnfStrCap);
	lAddNativeFunc(c,"substr",        "(s &start &stop)","Returns a copy of string s starting at position &start=0 and ending at &stop=(str-len s)",lnfSubstr);
	lAddNativeFunc(c,"str->sym",      "(s)",             "Converts string s to a symbol",                                                           lnfStrSym);
	lAddNativeFunc(c,"sym->str",      "(s)",             "Converts symbol s to a string",                                                           lnfSymStr);

	lAddNativeFunc(c,"ansi-reset","()",  "Ansi reset code",                 lnfAnsiRS);
	lAddNativeFunc(c,"ansi-fg",   "(a)", "Returns Ansi fg color code for a",lnfAnsiFG);
	lAddNativeFunc(c,"br",        "(&a)","Returns &a=1 linebreaks",         lnfBr);
}
