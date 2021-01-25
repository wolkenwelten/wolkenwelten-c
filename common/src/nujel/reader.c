#include "reader.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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
	return NULL;
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
	if(*s->data++ != '#'){return NULL;}
	switch(*s->data++){
	default:
	case 'n': s->data+=2; return NULL;
	case 't':
		return lValBool(true);
	case 'f':
		return lValBool(false);
	case 'i':
		return lValInf();
	case '(':
		return lCons(lValSymS(symArr),lReadCString(s));
	}

}

lVal *lReadCString(lCString *s){
	lVal *v, *ret;
	ret = v = lCons(NULL,NULL);
	while(1){
		lStringAdvanceToNextCharacter(s);
		char c = *s->data;
		if((c == 0) || (c == ')') || (s->data >= s->bufEnd)){
			s->data++;
			return ret;
		}

		if(v->vList.car != NULL){
			v->vList.cdr = lCons(NULL,NULL);
			v = v->vList.cdr;
		}

		if(c == '('){
			s->data+=1;
			v->vList.car = lReadCString(s);
		}else if(c == '\''){
			s->data++;
			if(*s->data == '('){
				s->data++;
				v->vList.car = lCons(lValSymS(symQuote),lCons(lReadCString(s),NULL));
			}else{
				v->vList.car = lCons(lValSymS(symQuote),lCons(lParseSymbol(s),NULL));
			}
		}else if(c == '"'){
			s->data++;
			v->vList.car = lParseString(s);
		}else if(isdigit(c)){
			v->vList.car = lParseNumber(s);
		}else if(c == '#'){
			v->vList.car = lParseSpecial(s);
		}else if((c == '-') && (isdigit(s->data[1]))){
			v->vList.car = lParseNumber(s);
		}else{
			v->vList.car = lParseSymbol(s);
		}
	}
}

lVal *lRead(const char *str){
	lCString *s = lCStringAlloc();
	s->data     = str;
	int len     = strlen(str);
	s->bufEnd   = &str[len];
	lVal *ret   = lReadCString(s);
	lCStringFree(s);
	return ret;
}
