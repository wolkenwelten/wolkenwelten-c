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
#include "../gui/widgetDrawing.h"
#include "../misc/lisp.h"
#include "../sdl/sdl.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

widget *widgetFocused = NULL;

#define WIDGET_COUNT (1<<12)
widget  widgetList[WIDGET_COUNT];
int     widgetMax = 1;
widget *widgetFirstFree = NULL;

static bool mouseInBox(uint x, uint y, uint w, uint h){
	if(mouseHidden) {return false;}
	if(mousex < x  ){return false;}
	if(mousex > x+w){return false;}
	if(mousey < y  ){return false;}
	if(mousey > y+h){return false;}
	return true;
}

widget *widgetGet(uint i){
	if(i >= WIDGET_COUNT){return NULL;}
	return &widgetList[i];
}

void lGUIWidgetFree(lVal *v){
	if(v->type != ltGUIWidget){return;}
	widgetFree(widgetGet(v->vCdr));
}

void widgetExport(widget *w, const char *symbol){
	lVal *v = lValAlloc();
	v->type = ltGUIWidget;
	v->vCdr = w - widgetList;
	lDefineVal(clRoot,symbol,v);
}

static lVal *getLArgW(lClosure *c, lVal *v, widget **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lEval(c,lCar(v));
	if((tlv != NULL) && (tlv->type == ltGUIWidget)){
		*res = widgetGet(tlv->vCdr);
	}
	return lCdr(v);
}

static lVal *lValW(widget *w){
	if(w == NULL){return NULL;}
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltGUIWidget;
	ret->vCdr = w - widgetList;
	return ret;
}

lVal *wwlnfWidgetNew(lClosure *c, lVal *v){
	int type = -1;

	v = getLArgI(c,v,&type);
	if(type < 0){return NULL;}
	widget *w = widgetNew(type);
	lVal *ret = lValW(w);
	if(ret == NULL){widgetFree(w);}

	return ret;
}

lVal *wwlnfWidgetParent(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}

	return lValW(w->parent);
}

lVal *wwlnfWidgetSetParent(lClosure *c, lVal *v){
	widget *w = NULL,*newParent = NULL;
	v = getLArgW(c,v,&w);
	v = getLArgW(c,v,&newParent);
	if((w == NULL) || (newParent == NULL)){return NULL;}
	widgetChild(newParent,w);

	return NULL;
}

lVal *wwlnfWidgetX(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->x);
}

lVal *wwlnfWidgetSetX(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->x = nv;
	return NULL;
}

lVal *wwlnfWidgetY(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->y);
}

lVal *wwlnfWidgetSetY(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->y = nv;
	return NULL;
}

lVal *wwlnfWidgetW(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->w);
}

lVal *wwlnfWidgetSetW(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->w = nv;
	return NULL;
}

lVal *wwlnfWidgetH(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->h);
}

lVal *wwlnfWidgetSetH(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->h = nv;
	return NULL;
}

lVal *wwlnfWidgetFlags(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->flags);
}

lVal *wwlnfWidgetSetFlags(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->flags = nv;
	return NULL;
}

lVal *wwlnfWidgetGX(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->gx);
}

lVal *wwlnfWidgetSetGX(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->gx = nv;
	return NULL;
}

lVal *wwlnfWidgetGY(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->gy);
}

lVal *wwlnfWidgetSetGY(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->gy = nv;
	return NULL;
}

lVal *wwlnfWidgetGW(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->gw);
}

lVal *wwlnfWidgetSetGW(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->gw = nv;
	return NULL;
}

lVal *wwlnfWidgetGH(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValInt(w->gh);
}

lVal *wwlnfWidgetSetGH(lClosure *c, lVal *v){
	widget *w = NULL;
	int nv = INT_MIN;
	v = getLArgW(c,v,&w);
	v = getLArgI(c,v,&nv);
	if((w == NULL) || (nv == INT_MIN)){return NULL;}
	w->gh = nv;
	return NULL;
}

lVal *wwlnfWidgetLabel(lClosure *c, lVal *v){
	widget *w = NULL;
	v = getLArgW(c,v,&w);
	if(w == NULL){return NULL;}
	return lValString(w->label);
}

lVal *wwlnfWidgetSetLabel(lClosure *c, lVal *v){
	widget *w = NULL;
	const char *str;
	v = getLArgW(c,v,&w);
	v = getLArgS(c,v,&str);
	if((w == NULL) || (str == NULL)){return NULL;}
	widgetLabel(w,str);

	return NULL;
}

lVal *wwlnfWidgetBind(lClosure *c, lVal *v){
	widget *w = NULL;
	const char *str;
	lVal *lam;
	v = getLArgW(c,v,&w);
	v = getLArgS(c,v,&str);
	v = getLArgL(c,v,&lam);
	if((w == NULL) || (str == NULL) || (lam == NULL)){return NULL;}
	widgetBindL(w,str,lam);

	return NULL;
}

void widgetAddLispFunctions(lClosure *c){
	lAddNativeFunc(c,"widget-new",    "(type)","Create a new widget of TYPE",      wwlnfWidgetNew);
	lAddNativeFunc(c,"widget-parent", "(widget)","Return the parent of WIDGET",    wwlnfWidgetParent);
	lAddNativeFunc(c,"widget-parent!","(widget parent)","Set the PARENT of WIDGET",wwlnfWidgetSetParent);

	lAddNativeFunc(c,"widget-x",      "(widget)","Get the X position of WIDGET",   wwlnfWidgetX);
	lAddNativeFunc(c,"widget-x!",     "(widget x)","Set the X position of WIDGET", wwlnfWidgetSetX);
	lAddNativeFunc(c,"widget-y",      "(widget)","Get the Y position of WIDGET",   wwlnfWidgetY);
	lAddNativeFunc(c,"widget-y!",     "(widget y)","Set the Y position of WIDGET", wwlnfWidgetSetY);
	lAddNativeFunc(c,"widget-width",  "(widget)","Get the W position of WIDGET",   wwlnfWidgetW);
	lAddNativeFunc(c,"widget-width!", "(widget w)","Set the W position of WIDGET", wwlnfWidgetSetW);
	lAddNativeFunc(c,"widget-height", "(widget)","Get the H position of WIDGET",   wwlnfWidgetH);
	lAddNativeFunc(c,"widget-height!","(widget h)","Set the H position of WIDGET", wwlnfWidgetSetH);

	lAddNativeFunc(c,"widget-goal-x",      "(widget)","Get the X position of WIDGET",   wwlnfWidgetGX);
	lAddNativeFunc(c,"widget-goal-x!",     "(widget x)","Set the X position of WIDGET", wwlnfWidgetSetGX);
	lAddNativeFunc(c,"widget-goal-y",      "(widget)","Get the Y position of WIDGET",   wwlnfWidgetGY);
	lAddNativeFunc(c,"widget-goal-y!",     "(widget y)","Set the Y position of WIDGET", wwlnfWidgetSetGY);
	lAddNativeFunc(c,"widget-goal-w",      "(widget)","Get the W position of WIDGET",   wwlnfWidgetGW);
	lAddNativeFunc(c,"widget-goal-w!",     "(widget w)","Set the W position of WIDGET", wwlnfWidgetSetGW);
	lAddNativeFunc(c,"widget-goal-h",      "(widget)","Get the H position of WIDGET",   wwlnfWidgetGH);
	lAddNativeFunc(c,"widget-goal-h!",     "(widget h)","Set the H position of WIDGET", wwlnfWidgetSetGH);

	lAddNativeFunc(c,"widget-label",  "(widget)","get the label of WIDGET",         wwlnfWidgetLabel);
	lAddNativeFunc(c,"widget-label!", "(widget label)","Set the LABEL of WIDGET",   wwlnfWidgetSetLabel);
	lAddNativeFunc(c,"widget-flags",  "(widget)","Get the flags WIDGET",            wwlnfWidgetFlags);
	lAddNativeFunc(c,"widget-flags!", "(widget flags)","Set the FLAGS of WIDGET",   wwlnfWidgetSetFlags);
	lAddNativeFunc(c,"widget-bind",   "(widget event handler)","Binds HANDLER to be evaluated on EVENT for WIDGET", wwlnfWidgetBind);
}

widget *widgetNew(widgetType type){
	widget *wid;
	if(widgetFirstFree != NULL){
		wid = widgetFirstFree;
		widgetFirstFree = wid->next;
	}else{
		if(widgetMax >= WIDGET_COUNT){
			fprintf(stderr,"widget Overflow!!!\n");
			return NULL;

		}
		wid = &widgetList[widgetMax++];
	}
	memset(wid,0,sizeof(widget));
	wid->type = type;
	if(wid->type == wTextInput){
		wid->vals = calloc(1,256);
		widgetBind(wid,"focus",textInputFocus);
		widgetBind(wid,"blur",textInputBlur);
	}
	if(wid->type == wTextLog){
		wid->valss = calloc(1,256 * sizeof(char *));
	}
	return wid;
}
widget *widgetNewC(widgetType type,widget *p){
	widget *wid = widgetNew(type);
	if(wid == NULL){return NULL;}
	widgetChild(p,wid);
	return wid;
}
widget *widgetNewCP(widgetType type,widget *p, int x, int y, int w, int h){
	widget *wid = widgetNewC(type,p);
	if(wid == NULL){return NULL;}
	wid->x = x;
	wid->y = y;
	wid->w = w;
	wid->h = h;
	return wid;
}
widget *widgetNewCPL(widgetType type,widget *p, int x, int y, int w, int h, const char *label){
	widget *wid = widgetNewCP(type,p,x,y,w,h);
	if(wid == NULL){return NULL;}
	widgetLabel(wid,label);
	return wid;
}
widget *widgetNewCPLH(widgetType type,widget *p, int x, int y, int w, int h, const char *label,const char *eventName, void (*handler)(widget *)){
	widget *wid = widgetNewCPL(type,p,x,y,w,h,label);
	if(wid == NULL){return NULL;}
	widgetBind(wid,eventName,handler);
	return wid;
}

void widgetFree(widget *w){
	if(w == NULL){return;}
	if(w->child != NULL){
		widgetEmpty(w);
		w->child = NULL;
	}
	if(w->parent != NULL){
		if(w->parent->child == w){
			if(w->next != NULL){ w->next->prev    = w->prev; }
			if(w->prev != NULL){ w->parent->child = w->next; }
		}else{
			if(w->prev != NULL){ w->prev->next = w->next; }
			if(w->next != NULL){ w->next->prev = w->prev; }
		}
		w->parent = w->next = w->prev = NULL;
	}
	if(w->type == wTextInput){free(w->vals); w->vals = NULL;}
	if(w->label != NULL){free((void *)w->label); w->label = NULL;}
	w->next = widgetFirstFree;
	widgetFirstFree = w;
}

void widgetEmpty(widget *w){
	widget *n = NULL;
	if(w == NULL){return;}
	if(w == w->child){return;}
	for(widget *c=w->child;c!=NULL;c=n){
		n = c->next;
		widgetFree(c);
	}
	w->child = NULL;
}

void widgetChild(widget *parent, widget *child){
	if(child->parent != NULL){
		child->parent->child = child->next;
		child->next = NULL;
		child->prev = NULL;
	}
	if(parent == NULL){ return; }
	child->parent = parent;
	if(parent->child != NULL){
		if(child->next != NULL){
			fprintf(stderr,"Widget already has a sibling!\n");
		}
		widget *c;
		for(c = parent->child;c->next != NULL;c = c->next){}
		c->next = child;
		child->prev = c;
	}else{
		parent->child = child;
	}
}

void widgetChildPre(widget *parent, widget *child){
	if(child->parent != NULL){
		child->parent->child = child->next;
		child->next = NULL;
		child->prev = NULL;
	}
	if(parent->child != NULL){
		if(child->next != NULL){
			fprintf(stderr,"Widget already has a sibling!\n");
		}
		child->next = parent->child;
		child->next->prev = child;
		child->prev = NULL;
	}
	parent->child = child;
	child->parent = parent;
}

void widgetFocus(widget *w){
	if(w == widgetFocused){return;}
	if(widgetFocused != NULL){
		widgetEmit(widgetFocused,"blur");
	}
	widgetFocused = w;
	if(w == NULL){return;}
	widgetEmit(w,"focus");
}

static int widgetIsSelectable(const widget *cur){
	if(cur == NULL){return 0;}
	if(cur->flags & WIDGET_HNS){return 0;}
	switch(cur->type){
		case wButton:
		case wButtonDel:
		case wRadioButton:
		case wTextInput:
		case wTextScroller:
		case wSlider:
		case wItemSlot:
		case wRecipeSlot:
			return 1;
		default:
			return 0;
	}
}

widget *widgetNextSel(const widget *cur){
	if(cur == NULL){return NULL;}

	if(!(cur->flags & WIDGET_HNS) && (cur->child != NULL)){
		if(widgetIsSelectable(cur->child)){return cur->child;}
		widget *ret = widgetNextSel(cur->child);
		if(ret != NULL){return ret;}
	}
	if(cur->next != NULL){
		if(widgetIsSelectable(cur->next)){return cur->next;}
		widget *ret = widgetNextSel(cur->next);
		if(ret != NULL){return ret;}
	}
	for(widget *c=cur->parent;c!=NULL;c=c->parent){
		if(widgetIsSelectable(c->next)){return c->next;}
		widget *ret = widgetNextSel(c->next);
		if(ret != NULL){return ret;}
	}
	return NULL;
}

widget *widgetPrevSel(const widget *cur){
	if(cur == NULL){return NULL;}

	if(!(cur->flags & WIDGET_HNS) && (cur->child != NULL)){
		widget *last=NULL;
		for(last=cur->child;last->next != NULL;last=last->next){}
		if(widgetIsSelectable(last)){return last;}
		if(last == cur->child)      {return last;}
		widget *ret = widgetPrevSel(last);
		if(ret != NULL){return ret;}
	}
	if(cur->prev != NULL){
		if(widgetIsSelectable(cur->prev)){
			return cur->prev;
		}
		widget *ret = widgetPrevSel(cur->prev);
		if(ret != NULL){return ret;}
	}
	for(widget *c=cur->parent;c!=NULL;c=c->parent){
		widget *last=NULL;
		for(last=c;last->next != NULL;last=last->next){}
		if(widgetIsSelectable(last)){return last;}
		if(last == c)               {return last;}
		widget *ret = widgetPrevSel(last);
		if(ret != NULL){return ret;}
	}
	return NULL;
}

void widgetLayVert(widget *w, int padding){
	int y = padding;
	for(widget *c=w->child;c!=NULL;c=c->next){
		c->y = y;
		y += c->h + padding;
	}
}

void widgetBind(widget *w, const char *eventName, void (*handler)(widget *)){
	eventHandler *h = calloc(1,sizeof(eventHandler));
	h->eventName    = eventName;
	h->lisp         = false;
	h->handler      = handler;
	h->next         = w->firstHandler;
	w->firstHandler = h;
}

void widgetBindL(widget *w, const char *eventName, lVal *handler){
	eventHandler *h = calloc(1,sizeof(eventHandler));
	h->eventName    = eventName;
	h->lisp         = true;
	h->lispHandler  = handler;
	h->next         = w->firstHandler;
	w->firstHandler = h;
}

int widgetEmit(widget *w, const char *eventName){
	int ret = 0;
	for(eventHandler *h=w->firstHandler;h!=NULL;h=h->next){
		if(strcmp(eventName,h->eventName) != 0){continue;}
		if(h->lisp){
			lispEvalL(lCons(lCons(h->lispHandler,lCons(lValW(w),lCons(lValString(eventName),NULL))),NULL));
		}else{
			h->handler(w);
		}
		ret++;
	}
	return ret;
}

static void widgetCheckEvents(widget *wid, int x, int y, int w, int h){
	if(wid == NULL){return;}
	if((wid->type == wSpace) || (wid->type == wPanel)){return;}
	if(mouseInBox(x,y,w,h)){
		if(!(wid->flags & WIDGET_HOVER)){widgetEmit(wid,"hover");}
		wid->flags |= WIDGET_HOVER;
		if(mouseClicked[0]){
			wid->flags |= WIDGET_CLICKED;
			if(wid->type == wSlider){
				float v = (float)(mousex - x) / (float)w;
				wid->vali = v*4096;
				widgetEmit(wid,"change");
				widgetFocus(wid);
			}
		}else{
			if(wid->flags & WIDGET_CLICKED){
				if((wid->type == wButtonDel) && ((int)mousex > x+w-40)){
					widgetEmit(wid,"altclick");
				}else if(widgetIsSelectable(wid)){
					widgetFocus(wid);
					widgetEmit(wid,"click");
				}
			}
			wid->flags &= ~WIDGET_CLICKED;
		}
		if(mouseClicked[2]){
			wid->flags |= WIDGET_ALT_CLICKED;
		}else{
			if((wid->flags & WIDGET_ALT_CLICKED) && widgetIsSelectable(wid)){
				widgetFocus(wid);
				widgetEmit(wid,"altclick");
			}
			wid->flags &= ~WIDGET_ALT_CLICKED;
		}
		if(mouseClicked[1]){
			wid->flags |= WIDGET_MID_CLICKED;
		}else{
			if((wid->flags & WIDGET_MID_CLICKED) && widgetIsSelectable(wid)){
				widgetFocus(wid);
				widgetEmit(wid,"midclick");
			}
			wid->flags &= ~WIDGET_MID_CLICKED;
		}
	}else{
		if(wid->flags & WIDGET_HOVER){widgetEmit(wid,"blur");}
		wid->flags &= ~(WIDGET_HOVER | WIDGET_CLICKED);
	}
}

static void widgetAnimate(widget *wid){
	if(!(wid->flags & WIDGET_ANIMATE)){return;}

	if(wid->flags & WIDGET_ANIMATEX){
		const int d = MAX(1,(abs(wid->x - wid->gx))>>3);
		if(wid->x < wid->gx){
			wid->x+=d;
		}else if(wid->x > wid->gx){
			wid->x-=d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEX;
		}
	}
	if(wid->flags & WIDGET_ANIMATEY){
		const int d = MAX(1,(abs(wid->y - wid->gy))>>3);
		if(wid->y < wid->gy){
			wid->y+=d;
		}else if(wid->y > wid->gy){
			wid->y-=d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEY;
		}
	}
	if(wid->flags & WIDGET_ANIMATEW){
		const int d = MAX(1,(abs(wid->w - wid->gw))>>3);
		if(wid->w < wid->gw){
			wid->w+=d;
		}else if(wid->w > wid->gw){
			wid->w-=d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEW;
		}
	}
	if(wid->flags & WIDGET_ANIMATEH){
		const int d = MAX(1,(abs(wid->h - wid->gh))>>3);
		if(wid->h < wid->gh){
			wid->h+=d;
		}else if(wid->h > wid->gh){
			wid->h-=d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEH;
		}
	}
}

void widgetDraw(widget *wid, textMesh *m,int px, int py, int pw, int ph){
	if(wid == NULL){return;}
	if(wid->flags & WIDGET_HIDDEN){return;}
	int x = px + wid->x;
	int y = py + wid->y;
	int w = wid->w;
	int h = wid->h;
	if(wid->x < 0){
		x = (px + pw) - wid->w + (wid->x+1);
	}
	if(wid->y < 0){
		y = (py + ph) - wid->h + (wid->y+1);
	}
	if(w < 0){ w = pw+(wid->w+1); }
	if(h < 0){ h = ph+(wid->h+1); }
	widgetCheckEvents(wid,x,y,w,h);
	widgetAnimate(wid);
	widgetDrawSingle(wid,m,x,y,w,h);
	for(widget *c=wid->child;c!=NULL;c=c->next){
		widgetDraw(c,m,x,y,w,h);
	}
}

void widgetFinish(widget *w){
	if(w->flags & WIDGET_ANIMATEW){w->w = w->gw;}
	if(w->flags & WIDGET_ANIMATEH){w->h = w->gh;}
	if(w->flags & WIDGET_ANIMATEX){w->x = w->gx;}
	if(w->flags & WIDGET_ANIMATEY){w->y = w->gy;}
	w->flags &= ~WIDGET_ANIMATE;
}

void widgetSlideW(widget *w, int nw){
	w->gw = nw;
	if(w->w == nw){return;}
	if(nw == 0){
		w->flags &= ~WIDGET_HIDDEN;
		w->flags |= WIDGET_ANIMATEW | WIDGET_NOSELECT;
	}else{
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEW;
	}
}

void widgetSlideH(widget *w, int nh){
	w->gh = nh;
	if(w->h == nh){return;}
	if(nh == 0){
		w->flags &= ~WIDGET_HIDDEN;
		w->flags |= WIDGET_ANIMATEH | WIDGET_NOSELECT;
	}else{
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEH;
	}
}

void widgetSlideX(widget *w, int nx){
	w->gx = nx;
	if(w->x == nx){return;}
	w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
	w->flags |= WIDGET_ANIMATEX;
}

void widgetSlideY(widget *w, int ny){
	w->gy = ny;
	if(w->y == ny){return;}
	w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
	w->flags |= WIDGET_ANIMATEY;
}

void widgetAddEntry(widget *w, const char *entry){
	if(w == NULL)          {return;}
	if(w->type != wTextLog){return;}
	if(w->valss == NULL)   {return;}
	if(entry == NULL)      {return;}
	if(*entry == 0)        {return;}
	if(w->valss[255] != NULL){free(w->valss[255]);}
	for(int i=255;i>0;i--){
		w->valss[i] = w->valss[i-1];
	}
	int len = strlen(entry);
	w->valss[0] = malloc(len+1);
	memcpy(w->valss[0],entry,len);
	w->valss[0][len] = 0;
}

void widgetLabel(widget *w, const char *newLabel){
	if(w == NULL){return;}
	if(newLabel == NULL){
		if(w->label != NULL){
			free((void *)w->label);
			w->label = NULL;
		}
		return;
	}
	size_t len = strlen(newLabel)+1;
	char *buf = malloc(len);
	snprintf(buf,len,"%s",newLabel);
	w->label = buf;
}
