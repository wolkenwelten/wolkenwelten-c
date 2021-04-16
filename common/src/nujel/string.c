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
	char *out = buf;
	char *end = &buf[len-2];
	if(buf >= end){return 0;}
	*out++ = '\"';
	while(out < end){
		switch(c = *data++){
		case 0:
			goto bufWriteStringExit;
		case '\a': // Bell
			*out++ = '\\'; *out++ = '\a';
			break;
		case '\b': // Backspace
			*out++ = '\\'; *out++ = '\b';
			break;
		case '\t': // Horiz. Tab
			*out++ = '\\'; *out++ = '\t';
			break;
		case '\n': // Line Feed
			*out++ = '\\'; *out++ = '\n';
			break;
		case '\v': // Vert. Tab
			*out++ = '\\'; *out++ = '\v';
			break;
		case '\f': // Form Feed
			*out++ = '\\'; *out++ = '\f';
			break;
		case '\r': // Carriage Return
			*out++ = '\\'; *out++ = '\r';
			break;
		case '\e': // Escape
			*out++ = '\\'; *out++ = '\e';
			break;
		case '"':
			*out++ = '\\'; *out++ = '"';
			break;
		case '\'':
			*out++ = '\\'; *out++ = '\'';
			break;
		case '\\':
			*out++ = '\\'; *out++ = '\\';
			break;
		default:
			*out++ = c;
			break;
		}
	}
	bufWriteStringExit:
	*out++ = '\"';
	return out-buf;
}

char *lSWriteVal(lVal *v, char *buf, char *bufEnd, int indentLevel, bool display){
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
			t = snprintf(cur,bufEnd-cur,"(ω ");
			indentLevel += 2;
		}else if(cl->flags & lfDynamic){
			t = snprintf(cur,bufEnd-cur,"(δ (");
			indentLevel += 3;
		}else{
			t = snprintf(cur,bufEnd-cur,"(λ (");
			indentLevel += 3;
		}
		if(t > 0){cur += t;}
		lVal *cloData = lCloData(v->vCdr);
		forEach(n,cloData){
			if(lCaar(n) == NULL){continue;}
			if(n != cloData){
				if(cl->flags & lfObject){
					*cur++ = '\n';
					for(int i=indentLevel;i>=0;i--){*cur++=' ';}
				}else{
					*cur++ = ' ';
				}
			}
			lVal *cv = NULL;
			if(lCadar(n) != NULL){
				cv = lCar(n);
			}else{
				cv = lCaar(n);
			}
			if(cl->flags & lfObject){
				cv = lCons(lValSym("def"),cv);
			}
			cur = lSWriteVal(cv,cur,bufEnd,indentLevel,display);
		}
		if(!(cl->flags & lfObject)){*cur++ = ')';}
		lVal *cloText = lCloText(v->vCdr);
		forEach(n,cloText){
			*cur++ = '\n';
			for(int i=indentLevel;i>=0;i--){*cur++=' ';}
			cur = lSWriteVal(lCar(n),cur,bufEnd,indentLevel,display);
		}
		indentLevel -= 2;
		t = snprintf(cur,bufEnd-cur,")");
		break; }
	case ltPair: {
		int indentStyle = 0;
		int oldIndent = indentLevel;
		lVal *carSym = lCar(v);
		if((carSym != NULL) && (carSym->type == ltSymbol) && (lCdr(v) != NULL)){
			if(lSymEq(&carSym->vSymbol,&symQuote) == 0){
				v = lCdar(v);
				*cur++ = '\'';
			}else if(lSymEq(&carSym->vSymbol,&symCond) == 0){
				indentStyle = 1;
				indentLevel += 6;
			}else if(lSymEq(&carSym->vSymbol,&symWhen) == 0){
				indentStyle = 1;
				indentLevel += 6;
			}else if(lSymEq(&carSym->vSymbol,&symUnless) == 0){
				indentStyle = 1;
				indentLevel += 8;
			}else if(lSymEq(&carSym->vSymbol,&symIf) == 0){
				indentStyle = 1;
				indentLevel += 4;
			}else if(lSymEq(&carSym->vSymbol,&symLet) == 0){
				indentStyle = 1;
				indentLevel += 5;
			}
		}
		t = snprintf(cur,bufEnd-cur,"(");
		if(t > 0){cur += t;}
		for(lVal *n = v;n != NULL; n = lCdr(n)){
			if(n->type == ltPair){
				cur = lSWriteVal(lCar(n),cur,bufEnd,indentLevel,display);
				if(lCdr(n) != NULL){
					if((indentStyle == 1) && (n != v)){
						*cur++ = '\n';
						for(int i=indentLevel;i>=0;i--){*cur++=' ';}
					}else{
						*cur++ = ' ';
					}
				}
			}else{
				*cur++ = '.';
				*cur++ = ' ';
				cur = lSWriteVal(n,cur,bufEnd,indentLevel,display);
				break;
			}
		}
		t = snprintf(cur,bufEnd-cur,")");
		indentLevel = oldIndent;
		break; }
	case ltArray: {
		t = snprintf(cur,bufEnd-cur,"#(");
		if(t > 0){cur += t;}
		if(lArrData(v) != NULL){
			const int arrLen = lArrLength(v);
			for(int i=0;i<arrLen;i++){
				cur = lSWriteVal(lValD(lArrData(v)[i]),cur,bufEnd,indentLevel,display);
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
		t += bufPrintFloat(lVecV(v->vCdr).x,&buf[t],t,len);
		buf[t++] = ' ';
		t += bufPrintFloat(lVecV(v->vCdr).y,&buf[t],t,len);
		buf[t++] = ' ';
		t += bufPrintFloat(lVecV(v->vCdr).z,&buf[t],t,len);
		t += snprintf(&buf[t],len,")");
		break;
	case ltString:
		if(display){
			t = snprintf(buf,len,"%s",lStrData(v));
		}else{
			t = bufWriteString(buf,len,lStrData(v));
		}
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
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	return lValInt(lStringLength(&lStr(t)));
}

lVal *lnfTrim(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const char *s;
	for(s = lStrData(t);*s != 0 && isspace((u8)*s);s++){}
	int len = lStringLength(&lStr(t)) - (s -  lStrData(t));
	for(;len > 0 && isspace((u8)s[len-1]);len--){}
	char *buf = malloc(len+1);
	memcpy(buf,s,len);
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

lVal *lnfStrDown(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *t = lEval(c,lCar(v));
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
	lVal *t = lEval(c,lCar(v));
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
	lVal *t = lEval(c,lCar(v));
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
	lVal *str = lEval(c,lCar(v));
	if(str == NULL){return NULL;}
	switch(str->type){
	default: return NULL;
	case ltString:
		if(str->vCdr == 0){return NULL;}
		buf  = lStrData(str);
		slen = len = lStringLength(&lStr(str));
		break;
	}
	if(lCdr(v) != NULL){
		v = lCdr(v);
		lVal *lStart = lEval(c,lCar(v));
		if(lStart->type == ltInt){
			start = lStart->vInt;
		}
		if(lCdr(v) != NULL){
			v = lCdr(v);
			lVal *lLen = lEval(c,lCar(v));
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
		lVal *t = lEval(c,lCar(sexpr));
		int clen = 0;
		if(t == NULL){continue;}
		switch(t->type){
		default: break;
		case ltInf: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"#inf");
			break; }
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
		case ltBool: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",t->vBool ? "#t" : "#f");
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

lVal *lnfIndexOf(lClosure *c, lVal *v){
	const char *haystack = NULL;
	const char *needle = NULL;
	int pos = 0;

	v = getLArgS(c,v,&haystack);
	v = getLArgS(c,v,&needle);
	v = getLArgI(c,v,&pos);

	if(needle == NULL)   {return lValInt(-1);}
	if(haystack == NULL) {return lValInt(-1);}
	const int needleLength = strlen(needle);
	if(needleLength <= 0){return lValInt(pos);}

	for(const char *s = &haystack[pos]; *s != 0; s++){
		if(strncmp(s,needle,needleLength) == 0){
			return lValInt(s-haystack);
		}
	}
	return lValInt(-1);
}

lVal *lnfAnsiFG(lClosure *c, lVal *v){
	int i = 0;
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,lCar(v)));
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
	v = lEval(c,lCar(v));
	if(v == NULL){return NULL;}
	if(v->type != ltString){return NULL;}
	return lValSym(lStrData(v));
}

lVal *lnfSymStr(lClosure *c, lVal *v){
	v = lEval(c,lCar(v));
	if(v == NULL){return NULL;}
	if(v->type != ltSymbol){return NULL;}
	return lValString(v->vSymbol.c);
}

lVal *lnfWriteStr(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	if(t == NULL){
		return lValString("#nil");
	}
	char *buf = malloc(1<<19);
	lSWriteVal(lCar(t), buf, &buf[1<<19],0,false);
	buf[(1<<19)-1]=0;
	t = lValString(buf);
	free(buf);
	return t;
}

lVal *lnfCharAt(lClosure *c,lVal *v){
	const char *str = NULL;
	int pos = 0;

	v = getLArgS(c,v,&str);
	v = getLArgI(c,v,&pos);

	if(str == NULL){return NULL;}
	const int len = strlen(str);
	if(pos >= len){return NULL;}
	return lValInt(str[pos]);
}

lVal *lnfFromCharCode(lClosure *c,lVal *v){
	int len = lListLength(v)+1;
	char *buf = malloc(len);
	int i=0,code;

	while(v != NULL){
		v = getLArgI(c,v,&code);
		buf[i++] = code;
		if(i >= len){break;}
	}
	v = lValString(buf);
	free(buf);
	return v;
}

void lAddStringFuncs(lClosure *c){
	lAddNativeFunc(c,"cat",           "(...args)",       "ConCATenates ...ARGS into a single string",                                                     lnfCat);
	lAddNativeFunc(c,"trim",          "(str)",           "Trims STR of any excessive whitespace",                                                         lnfTrim);
	lAddNativeFunc(c,"str-len",       "(str)",           "Returns length of string STR",                                                                  lnfStrlen);
	lAddNativeFunc(c,"str-up",        "(str)",           "Returns a copy of string STR all uppercased",                                                   lnfStrUp);
	lAddNativeFunc(c,"str-down",      "(str)",           "Returns a copy of string STR all lowercased",                                                   lnfStrDown);
	lAddNativeFunc(c,"str-capitalize","(str)",           "Returns a copy of string STR capitalized",                                                      lnfStrCap);
	lAddNativeFunc(c,"substr",        "(str &start &stop)","Returns a copy of string STR starting at position &start=0 and ending at &stop=(str-len s)",  lnfSubstr);
	lAddNativeFunc(c,"index-of",      "(haystack needle &start)","Returns the position of NEEDLE in HAYSTACK, searcing from &START=0, or -1 if not found",lnfIndexOf);
	lAddNativeFunc(c,"char-at",       "(str pos)",       "Returns the character at position POS in STR",                                                  lnfCharAt);
	lAddNativeFunc(c,"from-char-code","(...codes)",      "Construct a string out of ...CODE codepoints and return it",                                    lnfFromCharCode);
	lAddNativeFunc(c,"str->sym",      "(str)",           "Converts STR to a symbol",                                                                      lnfStrSym);
	lAddNativeFunc(c,"sym->str",      "(sym)",           "Converts SYM to a string",                                                                      lnfSymStr);
	lAddNativeFunc(c,"write-str",     "(val)",           "Writes V into a string and returns it",                                                         lnfWriteStr);

	lAddNativeFunc(c,"ansi-reset","()",  "Ansi reset code",                 lnfAnsiRS);
	lAddNativeFunc(c,"ansi-fg",   "(a)", "Returns Ansi fg color code for a",lnfAnsiFG);
	lAddNativeFunc(c,"br",        "(&a)","Returns &a=1 linebreaks",         lnfBr);
}
