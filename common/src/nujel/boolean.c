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

#include "boolean.h"
#include "casting.h"

lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lnfBool(c,v);
	return lValBool(a == NULL ? true : !a->vBool);
}

lVal *lnfAnd(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(true);}
	lVal *t = lnfBool(c,v);
	if((t == NULL) || (!t->vBool)){return lValBool(false);}
	return lnfAnd(c,v->vList.cdr);
}

lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lnfBool(c,v);
	if((t != NULL) && t->vBool){return lValBool(true);}
	return lnfOr(c,v->vList.cdr);
}

void lAddBooleanFuncs(lClosure *c){
	lAddNativeFunc(c,"and &&","(...args)","#t if all ...args evaluate to true",            lnfAnd);
	lAddNativeFunc(c,"or ||" ,"(...args)","#t if one member of ...args evaluates to true", lnfOr);
	lAddNativeFunc(c,"not !","(a)",      "#t if a is #f, #f if a is #t",                  lnfNot);
}
