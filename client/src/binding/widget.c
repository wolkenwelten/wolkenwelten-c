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

#include "widget.h"


#include "../gui/menu.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../misc/lisp.h"
#include "../gfx/gfx.h"

#include "../../../common/nujel/lib/api.h"

#include <limits.h>

widget *castToWidget(lVal *v){
	if(v == NULL){return NULL;}
	if(v->type != ltGUIWidget){return NULL;}
	const int i = v->vInt;
	if(i < 0){return NULL;}
	if(i > widgetMax){return NULL;}
	return &widgetList[i];
}

lVal *lValW(widget *w){
	if(w == NULL){return NULL;}
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltGUIWidget;
	ret->vInt = w - widgetList;
	return ret;
}

widget *widgetGet(uint i){
	if(i >= WID_MAX){return NULL;}
	return &widgetList[i];
}

void widgetExport(widget *w, const char *symbol){
	lDefineVal(clRoot,symbol,lValW(w));
}

static lVal *wwlnfWidgetNew(lClosure *c, lVal *v){
	(void)c;
	const int type = castToInt(lCar(v),-1);
	if(type < 0){return NULL;}
	widget *w = widgetNew(type);
	widgetChild(rootMenu,w);
	lVal *ret = lValW(w);

	return ret;
}

static lVal *wwlnfWidgetParent(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValW(w->parent);
}

static lVal *wwlnfWidgetSetParent(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	if(w == NULL){return NULL;}
	widget *newParent = castToWidget(lCadr(v));
	widgetChild(newParent,w);

	return car;
}

static lVal *wwlnfWidgetFocusGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return (widgetFocused == NULL) ? NULL :lValW(widgetFocused);
}

static lVal *wwlnfWidgetFocusSet(lClosure *c, lVal *v){
	(void)c; (void)v;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	if(w == NULL){return NULL;}
	widgetFocus(w);
	return car;
}

static lVal *wwlnfWidgetX(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->area.x);
}

static lVal *wwlnfWidgetSetX(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	const int nv = castToInt(lCadr(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->area.x = nv;
	return car;
}

static lVal *wwlnfWidgetY(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->area.y);
}

static lVal *wwlnfWidgetSetY(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	const int nv = castToInt(lCadr(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->area.y = nv;
	return car;
}

static lVal *wwlnfWidgetW(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->area.w);
}

static lVal *wwlnfWidgetSetW(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	const int nv = castToInt(lCadr(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->area.w = nv;
	return car;
}

static lVal *wwlnfWidgetH(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->area.h);
}

static lVal *wwlnfWidgetSetH(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	const int nv = castToInt(lCadr(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->area.h = nv;
	return car;
}

static lVal *wwlnfWidgetFlags(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->flags);
}

static lVal *wwlnfWidgetSetFlags(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	if((car == NULL) || (car->type != ltGUIWidget)){
		lExceptionThrowValClo("type-error", "Expected a widget", car, c);
	}
	lVal *cadr = lCadr(v);
	if((cadr == NULL) || (cadr->type != ltInt)){
		lExceptionThrowValClo("type-error", "Expected an :int", cadr, c);
	}
	w->flags = cadr->vInt;
	return car;
}

static lVal *wwlnfWidgetGX(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	if(w == NULL){return NULL;}
	return lValInt(w->goalArea.x);
}

static lVal *wwlnfWidgetSetGX(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	v = lCdr(v);
	int nv = castToInt(lCar(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->goalArea.x = nv;
	return car;
}

static lVal *wwlnfWidgetGY(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->goalArea.y);
}

static lVal *wwlnfWidgetSetGY(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	v = lCdr(v);
	const int nv = castToInt(lCar(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->goalArea.y = nv;
	return car;
}

static lVal *wwlnfWidgetGW(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	if(w == NULL){return NULL;}
	return lValInt(w->goalArea.w);
}

static lVal *wwlnfWidgetSetGW(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	v = lCdr(v);
	int nv = castToInt(lCar(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->goalArea.w = nv;
	return car;
}

static lVal *wwlnfWidgetGH(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValInt(w->goalArea.h);
}

static lVal *wwlnfWidgetSetGH(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	v = lCdr(v);
	const int nv = castToInt(lCar(v),INT_MIN);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->goalArea.h = nv;
	return car;
}

static lVal *wwlnfWidgetLabel(lClosure *c, lVal *v){
	(void)c;
	widget *w = castToWidget(lCar(v));
	return w == NULL ? NULL : lValString(w->label);
}

static lVal *wwlnfWidgetSetLabel(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	v = lCdr(v);
	const char *str = castToString(lCar(v),NULL);
	if((w == NULL) || (str == NULL)){return NULL;}
	widgetLabel(w,str);
	return car;
}

static lVal *wwlnfWidgetBind(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	v = lCdr(v);
	const char *str = castToString(lCar(v),NULL); v = lCdr(v);
	lVal *callback = lCar(v);
	if((w == NULL) || (str == NULL) || (callback == NULL) || (callback->type != ltLambda)){
		return NULL;
	}
	widgetBindL(w,str,callback);
	return car;
}

lVal *wwlnfWidgetActiveCount(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(widgetActive);
}

lVal *wwlnfWidgetValInt(lClosure *c, lVal *v){
	(void)c; (void)v;
	widget *wid = castToWidget(lCar(v));
	if(wid == NULL){return NULL;}
	return lValInt(wid->vali);
}

lVal *wwlnfWidgetValIntSet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	widget *w = castToWidget(car);
	if(w == NULL){return NULL;}
	lVal *val = lCadr(v);
	if((val == NULL) || (val->type != ltInt)){return NULL;}

	w->vali = val->vInt;

	return car;
}

void lOperatorsWidget(lClosure *c){
	lAddNativeFunc(c,"widget/new",    "[type]",         "Create a new widget of type", wwlnfWidgetNew);
	lAddNativeFunc(c,"widget/parent", "[widget]",       "Return the parent of widget", wwlnfWidgetParent);
	lAddNativeFunc(c,"widget/parent!","[widget parent]","Set the parent of widget",    wwlnfWidgetSetParent);
	lAddNativeFunc(c,"widget/focus",  "[]",             "Return the focused widget",   wwlnfWidgetFocusGet);
	lAddNativeFunc(c,"widget/focus!", "[widget]",       "Focus widget",                wwlnfWidgetFocusSet);

	lAddNativeFunc(c,"widget/x",      "[widget]",  "Get the X position of WIDGET", wwlnfWidgetX);
	lAddNativeFunc(c,"widget/x!",     "[widget x]","Set the X position of WIDGET", wwlnfWidgetSetX);
	lAddNativeFunc(c,"widget/y",      "[widget]",  "Get the Y position of WIDGET", wwlnfWidgetY);
	lAddNativeFunc(c,"widget/y!",     "[widget y]","Set the Y position of WIDGET", wwlnfWidgetSetY);
	lAddNativeFunc(c,"widget/width",  "[widget]",  "Get the W position of WIDGET", wwlnfWidgetW);
	lAddNativeFunc(c,"widget/width!", "[widget w]","Set the W position of WIDGET", wwlnfWidgetSetW);
	lAddNativeFunc(c,"widget/height", "[widget]",  "Get the H position of WIDGET", wwlnfWidgetH);
	lAddNativeFunc(c,"widget/height!","[widget h]","Set the H position of WIDGET", wwlnfWidgetSetH);

	lAddNativeFunc(c,"widget/goal-x", "[widget]",  "Get the X position of WIDGET", wwlnfWidgetGX);
	lAddNativeFunc(c,"widget/goal-x!","[widget x]","Set the X position of WIDGET", wwlnfWidgetSetGX);
	lAddNativeFunc(c,"widget/goal-y", "[widget]",  "Get the Y position of WIDGET", wwlnfWidgetGY);
	lAddNativeFunc(c,"widget/goal-y!","[widget y]","Set the Y position of WIDGET", wwlnfWidgetSetGY);
	lAddNativeFunc(c,"widget/goal-w", "[widget]",  "Get the W position of WIDGET", wwlnfWidgetGW);
	lAddNativeFunc(c,"widget/goal-w!","[widget w]","Set the W position of WIDGET", wwlnfWidgetSetGW);
	lAddNativeFunc(c,"widget/goal-h", "[widget]",  "Get the H position of WIDGET", wwlnfWidgetGH);
	lAddNativeFunc(c,"widget/goal-h!","[widget h]","Set the H position of WIDGET", wwlnfWidgetSetGH);

	lAddNativeFunc(c,"widget/label",  "[widget]",      "get the label of WIDGET", wwlnfWidgetLabel);
	lAddNativeFunc(c,"widget/label!", "[widget label]","Set the LABEL of WIDGET", wwlnfWidgetSetLabel);
	lAddNativeFunc(c,"widget/flags",  "[widget]",      "Get the flags WIDGET",    wwlnfWidgetFlags);
	lAddNativeFunc(c,"widget/flags!", "[widget flags]","Set the FLAGS of WIDGET", wwlnfWidgetSetFlags);
	lAddNativeFunc(c,"widget/bind",   "[widget event handler]","Binds HANDLER to be evaluated on EVENT for WIDGET", wwlnfWidgetBind);

	lAddNativeFunc(c,"widget/val/int", "[widget]","Sets the vali field of WIDGET to VAL", wwlnfWidgetValInt);
	lAddNativeFunc(c,"widget/val/int!","[widget val]","Sets the vali field of WIDGET to VAL", wwlnfWidgetValIntSet);

	lAddNativeFunc(c,"widget/active-count", "[]","Return the amount of widgets that are currently allocated", wwlnfWidgetActiveCount);
}
