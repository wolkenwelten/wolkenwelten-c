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

#include "reader.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void lStringAdvanceToNextCharacter(lString *s){
	for(;(*s->data != 0) && (isspace((u8)*s->data));s->data++){}
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
			v->vString = lStringNew(buf,b-buf);
			return v;
		}else{
			*b++ = *s->data++;
		}
	}
	return NULL;
}

static lVal *lParseNumber(lString *s){
	lVal *v = lValInt(0);
	int c   = *s->data;
	int fc  = c;
	bool isFloat = false;
	int cval = 0;
	int digits = 0;
	if(fc == '-'){c = *++s->data;}
	while(!isspace((u8)c)){
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

static lVal *lParseSymbol(lString *s){
	lVal *v = lValAlloc();
	v->type = ltSymbol;
	v->vSymbol.v[0] = 0;
	v->vSymbol.v[1] = 0;
	for(int i=0;i<16;i++){
		char c = *s->data++;
		if(isspace((u8)c) || (c == ')') || (c ==0)){
			s->data--;
			break;
		}
		v->vSymbol.c[i] = c;
	}
	while(isspace((u8)*s->data)){
		if(*s->data == 0){break;}
		s->data++;
	}
	return v;
}

static lVal *lParseSpecial(lString *s){
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
		return lCons(lValSymS(symArr),lReadString(s));
	}

}

lVal *lReadString(lString *s){
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
			v->vList.car = lReadString(s);
		}else if(c == '\''){
			s->data++;
			if(*s->data == '('){
				s->data++;
				v->vList.car = lCons(lValSymS(symQuote),lCons(lReadString(s),NULL));
			}else{
				v->vList.car = lCons(lValSymS(symQuote),lCons(lParseSymbol(s),NULL));
			}
		}else if(c == '"'){
			s->data++;
			v->vList.car = lParseString(s);
		}else if(isdigit((u8)c)){
			v->vList.car = lParseNumber(s);
		}else if(c == '#'){
			v->vList.car = lParseSpecial(s);
		}else if((c == '-') && (isdigit((u8)s->data[1]))){
			v->vList.car = lParseNumber(s);
		}else{
			v->vList.car = lParseSymbol(s);
		}
	}
}

lVal *lRead(const char *str){
	lString *s = lStringAlloc();
	if(s == NULL){return NULL;}
	s->data     = str;
	s->buf      = str;
	s->bufEnd   = &str[strlen(str)];
	lVal *ret   = lReadString(s);
	return ret;
}
