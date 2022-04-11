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

#include "widgets/widgets.h"
#include "../binding/widget.h"
#include "../gui/menu.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../misc/lisp.h"
#include "../sdl/sdl.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"

#include "../../../common/nujel/lib/api.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

widget *widgetFocused = NULL;

widget  widgetList[WID_MAX];
int     widgetActive = 0;
int     widgetMax = 0;
widget *widgetFirstFree = NULL;

eventHandler  eventHandlerList[EVH_MAX];
int           eventHandlerActive;
int           eventHandlerMax;
eventHandler *eventHandlerFirstFree;

eventHandler *eventHandlerAlloc(){
	eventHandler *evh;
	if(eventHandlerFirstFree == NULL){
		if(eventHandlerMax >= EVH_MAX){
			lGarbageCollect();
			if(eventHandlerFirstFree == NULL){
				fprintf(stderr,"eventHandler Overflow!!!\n");
				exit(123);
			}else{
				evh = eventHandlerFirstFree;
				eventHandlerFirstFree = evh->next;
			}
		}else{
			evh = &eventHandlerList[eventHandlerMax++];
		}
	}else{
		evh = eventHandlerFirstFree;
		eventHandlerFirstFree = evh->next;
	}
	eventHandlerActive++;
	*evh = (eventHandler){0};
	return evh;
}

void eventHandlerFree(eventHandler *evh){
	if(evh == NULL){return;}
	evh->next = eventHandlerFirstFree;
	eventHandlerFirstFree = evh;
	eventHandlerActive--;
}

static bool mouseInArea(const box2D area){
	if(mouseHidden)              {return false;}
	if(mousex < area.x  )        {return false;}
	if(mousex > area.x + area.w) {return false;}
	if(mousey < area.y  )        {return false;}
	if(mousey > area.y + area.h) {return false;}
	return true;
}

widget *widgetAlloc(){
	widget *wid;
	if(widgetFirstFree == NULL){
		if(widgetMax >= WID_MAX){
			lGarbageCollect();
			if(widgetFirstFree == NULL){
				fprintf(stderr,"widget Overflow!!!\n");
				exit(123);
			}else{
				wid = widgetFirstFree;
				widgetFirstFree = wid->next;
			}
		}else{
			wid = &widgetList[widgetMax++];
		}
	}else{
		wid = widgetFirstFree;
		widgetFirstFree = wid->next;
	}
	widgetActive++;
	*wid = (widget){0};
	return wid;
}

void widgetFree(widget *w){
	if(w == NULL){return;}
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
	*w = (widget){0};
	w->next = widgetFirstFree;
	widgetFirstFree = w;
	widgetActive--;
}

widget *widgetNew(widgetType type){
	widget *wid = widgetAlloc();
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
widget *widgetNewCP(widgetType type,widget *p, box2D area){
	widget *wid = widgetNewC(type,p);
	if(wid == NULL){return NULL;}
	wid->area = area;
	return wid;
}
widget *widgetNewCPL(widgetType type,widget *p, box2D area, const char *label){
	widget *wid = widgetNewCP(type,p,area);
	if(wid == NULL){return NULL;}
	widgetLabel(wid,label);
	return wid;
}
widget *widgetNewCPLH(widgetType type,widget *p, box2D area, const char *label,const char *eventName, void (*handler)(widget *)){
	widget *wid = widgetNewCPL(type,p,area,label);
	if(wid == NULL){return NULL;}
	widgetBind(wid,eventName,handler);
	return wid;
}

void widgetEmpty(widget *w){
	if(w == NULL){return;}
	w->child = NULL;
}

void widgetChildRemove(widget *parent, widget *child){
	if(parent->child == child){
		parent->child = child->next;
	}
	for(widget *w = parent->child; w ; w = w->next){
		if(w->next == child){w->next = child->next;}
		if(w->prev == child){w->prev = child->prev;}
	}
	child->next = NULL;
	child->prev = NULL;
	child->parent = NULL;
}

void widgetChild(widget *parent, widget *child){
	if(child->parent != NULL){
		widgetChildRemove(child->parent,child);
	}
	if(parent == NULL){ return; }
	child->parent = parent;
	if(parent->child == NULL){
		parent->child = child;
	}else{
		widget *c;
		for(c = parent->child;c->next != NULL;c = c->next){}
		c->next = child;
		child->prev = c;
	}
}

void widgetChildPre(widget *parent, widget *child){
	if(child->parent != NULL){widgetChildRemove(child->parent,child);}
	if(parent->child != NULL){
		child->next = parent->child;
		child->next->prev = child;
	}
	parent->child = child;
	child->parent = parent;
}

void widgetFocus(widget *w){
	if(w == widgetFocused){return;}
	widgetFocused = w;
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
		c->area.y = y;
		y += c->area.h + padding;
	}
}

void widgetBind(widget *w, const char *eventName, void (*handler)(widget *)){
	eventHandler *h = eventHandlerAlloc();
	h->eventName    = eventName;
	h->lisp         = false;
	h->handler      = handler;
	h->next         = w->firstHandler;
	w->firstHandler = h;
}

void widgetBindL(widget *w, const char *eventName, lVal *handler){
	eventHandler *h = eventHandlerAlloc();
	h->eventName    = eventName;
	h->lisp         = true;
	h->lispHandler  = handler;
	h->next         = w->firstHandler;
	w->firstHandler = h;
}

int widgetEmit(widget *w, const char *eventName){
	int ret = 0;
	if(w == NULL){return 0;}
	for(eventHandler *h=w->firstHandler;h!=NULL;h=h->next){
		if(strcmp(eventName,h->eventName) != 0){continue;}
		if(h->lisp){
			lVal *form = lRootsValPush(lCons(h->lispHandler,NULL));
			lVal *l = lCons(NULL,NULL);
			form->vList.cdr = l;
			l->vList.car = lValW(w);
			l->vList.cdr = lCons(NULL,NULL);
			l = l->vList.cdr;
			l->vList.car = lValString(eventName);
			lispEvalL(form);
		}else{
			h->handler(w);
		}
		ret++;
	}
	return ret;
}

static void widgetAnimate(widget *wid){
	if(!(wid->flags & WIDGET_ANIMATE)){return;}

	if(wid->flags & WIDGET_ANIMATEX){
		const int d = MAX(1,(abs(wid->area.x - wid->goalArea.x)) >> 3);
		if(wid->area.x < wid->goalArea.x){
			wid->area.x += d;
		}else if(wid->area.x > wid->goalArea.x){
			wid->area.x -= d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEX;
		}
	}
	if(wid->flags & WIDGET_ANIMATEY){
		const int d = MAX(1,(abs(wid->area.y - wid->goalArea.y))>>3);
		if(wid->area.y < wid->goalArea.y){
			wid->area.y += d;
		}else if(wid->area.y > wid->goalArea.y){
			wid->area.y -= d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEY;
		}
	}
	if(wid->flags & WIDGET_ANIMATEW){
		const int d = MAX(1,(abs(wid->area.w - wid->goalArea.w))>>3);
		if(wid->area.w < wid->goalArea.w){
			wid->area.w += d;
		}else if(wid->area.w > wid->goalArea.w){
			wid->area.w -= d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEW;
		}
	}
	if(wid->flags & WIDGET_ANIMATEH){
		const int d = MAX(1,(abs(wid->area.h - wid->goalArea.h))>>3);
		if(wid->area.h < wid->goalArea.h){
			wid->area.h += d;
		}else if(wid->area.h > wid->goalArea.h){
			wid->area.h -= d;
		}else{
			wid->flags &= ~WIDGET_ANIMATEH;
		}
	}
}

box2D widgetCalcPosition(const widget *wid, const box2D parentArea){
	int x = parentArea.x + wid->area.x;
	int y = parentArea.y + wid->area.y;
	int w = wid->area.w;
	int h = wid->area.h;

	if(wid->area.x < 0){
		x = (parentArea.x + parentArea.w) - wid->area.w + (wid->area.x+1);
	}
	if(wid->area.y < 0){
		y = (parentArea.y + parentArea.h) - wid->area.h + (wid->area.y+1);
	}
	if(w < 0){ w = parentArea.w+(wid->area.w+1); }
	if(h < 0){ h = parentArea.h+(wid->area.h+1); }

	return rect(x,y,w,h);
}

box2D widgetCalcChildPosition(const widget *wid, const box2D area){
	switch(wid->type){
	default:
		return area;
	case wPanel:
		return rect(
			area.x + wid->vali,
			area.y + wid->vali,
			area.w - wid->vali * 2,
			area.h - wid->vali * 2
		);
	}
}

box2D widgetCalcChildPositionDirectly(const widget *wid, const box2D parentArea){
	return widgetCalcChildPosition(wid, widgetCalcPosition(wid, parentArea));
}

box2D widgetCalcPositionFromScratch(const widget *wid, const box2D screen){
	if(wid == NULL){return screen;}
	return widgetCalcChildPositionDirectly(wid, widgetCalcPositionFromScratch(wid->parent, screen));
}

widget *widgetDraw(widget *wid, textMesh *m,const box2D parentArea){
	if((wid == NULL) || (wid->flags & WIDGET_HIDDEN)){return NULL;}
	widget *ret = NULL;
	const box2D area = widgetCalcPosition(wid,parentArea);
	widgetAnimate(wid);
	widgetDrawSingle(wid,m,area);

	ret = mouseInArea(area) ? wid : NULL;
	const box2D childArea = widgetCalcChildPosition(wid,area);

	for(widget *c=wid->child;c!=NULL;c=c->next){
		widget *tmp = widgetDraw(c,m,childArea);
		if(tmp){ret = tmp;}
	}
	return ret;
}

bool widgetDoMouseEvents(widget *wid, widget *goal, box2D screen){
	if(wid == NULL){return false;}

	bool active = wid == goal;
	for(widget *c=wid->child;c!=NULL;c=c->next){
		active |= widgetDoMouseEvents(c,goal, screen);
	}

	if(active){
		if((wid->type == wSpace) || (wid->type == wPanel)){return active;}
		if(!(wid->flags & WIDGET_HOVER)){widgetEmit(wid,"hover");}
		wid->flags |= WIDGET_HOVER;
		if(mouseClicked[0]){
			wid->flags |= WIDGET_CLICKED;
			if(wid->type == wSlider){
				const box2D widArea = widgetCalcPositionFromScratch(wid, screen);
				float v = (float)(mousex - widArea.x) / (float)(widArea.w);
				wid->vali = v*4096;
				widgetEmit(wid,"change");
				widgetFocus(wid);
			}
		}else{
			if(wid->flags & WIDGET_CLICKED){
				const box2D widArea = widgetCalcPositionFromScratch(wid, screen);
				if((wid->type == wButtonDel) && ((int)mousex > widArea.x+widArea.w-40)){
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
		wid->flags &= ~(WIDGET_MID_CLICKED | WIDGET_ALT_CLICKED | WIDGET_CLICKED | WIDGET_HOVER);
	}
	return active;
}

void widgetFinish(widget *w){
	if(w->flags & WIDGET_ANIMATEW){w->area.w = w->goalArea.w;}
	if(w->flags & WIDGET_ANIMATEH){w->area.h = w->goalArea.h;}
	if(w->flags & WIDGET_ANIMATEX){w->area.x = w->goalArea.x;}
	if(w->flags & WIDGET_ANIMATEY){w->area.y = w->goalArea.y;}
	w->flags &= ~WIDGET_ANIMATE;
}

void widgetSlideW(widget *w, int nw){
	w->goalArea.w = nw;
	if(w->area.w == nw){return;}
	if(nw == 0){
		w->flags &= ~WIDGET_HIDDEN;
		w->flags |= WIDGET_ANIMATEW | WIDGET_NOSELECT;
	}else{
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEW;
	}
}

void widgetSlideH(widget *w, int nh){
	w->goalArea.h = nh;
	if(w->area.h == nh){return;}
	if(nh == 0){
		w->flags &= ~WIDGET_HIDDEN;
		w->flags |= WIDGET_ANIMATEH | WIDGET_NOSELECT;
	}else{
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEH;
	}
}

void widgetSlideX(widget *w, int nx){
	w->goalArea.x = nx;
	if(w->area.x == nx){return;}
	w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
	w->flags |= WIDGET_ANIMATEX;
}

void widgetSlideY(widget *w, int ny){
	w->goalArea.y = ny;
	if(w->area.y == ny){return;}
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

void widgetUpdateAllEvents(){
	static widget *lastFocused = NULL;
	if(widgetFocused == lastFocused){return;}
	widgetEmit(lastFocused,"blur");
	lastFocused = widgetFocused;
	widgetEmit(widgetFocused,"focus");
}
