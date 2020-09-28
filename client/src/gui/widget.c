#define _DEFAULT_SOURCE
#include "widget.h"

#include "../gui/menu.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../sdl/sdl.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

widget *widgetFocused = NULL;

static bool mouseInBox(uint x, uint y, uint w, uint h){
	if(mousex < x  ){return false;}
	if(mousex > x+w){return false;}
	if(mousey < y  ){return false;}
	if(mousey > y+h){return false;}
	return true;
}

widget *widgetNew(int type){
	widget *wid = calloc(1,sizeof(widget));
	if(wid == NULL){return NULL;}
	wid->type = type;
	if(wid->type == WIDGET_TEXTINPUT){
		wid->vals = calloc(1,256);
		widgetBind(wid,"focus",textInputFocus);
		widgetBind(wid,"blur",textInputBlur);
	}
	return wid;
}
widget *widgetNewC(int type,widget *p){
	widget *wid = widgetNew(type);
	if(wid == NULL){return NULL;}
	widgetChild(p,wid);
	return wid;
}
widget *widgetNewCP(int type,widget *p, int x, int y, int w, int h){
	widget *wid = widgetNewC(type,p);
	if(wid == NULL){return NULL;}
	wid->x = x;
	wid->y = y;
	wid->w = w;
	wid->h = h;
	return wid;
}
widget *widgetNewCPL(int type,widget *p, int x, int y, int w, int h, const char *label){
	widget *wid = widgetNewCP(type,p,x,y,w,h);
	if(wid == NULL){return NULL;}
	wid->label = label;
	return wid;
}
widget *widgetNewCPLH(int type,widget *p, int x, int y, int w, int h, const char *label,const char *eventName, void (*handler)(widget *)){
	widget *wid = widgetNewCPL(type,p,x,y,w,h,label);
	if(wid == NULL){return NULL;}
	widgetBind(wid,eventName,handler);
	return wid;
}

void widgetFree(widget *w){
	if(w == NULL){return;}
	if(w->child != NULL){
		widgetEmpty(w);
	}
	if(w->parent != NULL){
		if(w->parent->child == w){
			w->parent->child = w->next;
		}else{
			// ToDo
		}
	}
	if(w->type == WIDGET_TEXTINPUT){
		free(w->vals);
	}
	free(w);
}

void widgetEmpty(widget *w){
	widget *n=NULL;
	if(w == NULL){return;}
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

static int widgetIsSelectable(widget *cur){
	if(cur == NULL){return 0;}
	if(cur->flags & WIDGET_HNS      ){return 0;}
	if(cur->type == WIDGET_BUTTON   ){return 1;}
	if(cur->type == WIDGET_BUTTONDEL){return 1;}
	return 0;
}

widget *widgetNextSel(widget *cur){
	if(cur == NULL){return NULL;}

	if(!(cur->flags & WIDGET_HNS) && (cur->child != NULL)){
		if(widgetIsSelectable(cur->child)){
			return cur->child;
		}
		widget *ret = widgetNextSel(cur->child);
		if(ret != NULL){
			return ret;
		}
	}
	if(cur->next != NULL){
		if(widgetIsSelectable(cur->next)){
			return cur->next;
		}
		widget *ret = widgetNextSel(cur->next);
		if(ret != NULL){
			return ret;
		}
	}
	for(widget *c=cur->parent;c!=NULL;c=c->parent){
		if(widgetIsSelectable(c->next)){
			return c->next;
		}
		widget *ret = widgetNextSel(c->next);
		if(ret != NULL){
			return ret;
		}
	}
	return NULL;
}

widget *widgetPrevSel(widget *cur){
	if(cur == NULL){return NULL;}

	if(!(cur->flags & WIDGET_HNS) && (cur->child != NULL)){
		widget *last=NULL;
		for(last=cur->child;last->next != NULL;last=last->next){}
		if(widgetIsSelectable(last)){
			return last;
		}
		widget *ret = widgetPrevSel(last);
		if(ret != NULL){
			return ret;
		}
	}
	if(cur->prev != NULL){
		if(widgetIsSelectable(cur->prev)){
			return cur->prev;
		}
		widget *ret = widgetPrevSel(cur->prev);
		if(ret != NULL){
			return ret;
		}
	}
	for(widget *c=cur->parent;c!=NULL;c=c->parent){
		widget *last=NULL;
		for(last=c;last->next != NULL;last=last->next){}
		if(widgetIsSelectable(last)){
			return last;
		}
		widget *ret = widgetPrevSel(last);
		if(ret != NULL){
			return ret;
		}
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
	h->handler      = handler;
	h->next         = w->firstHandler;
	w->firstHandler = h;
}

void widgetEmit(widget *w, const char *eventName){
	for(eventHandler *h=w->firstHandler;h!=NULL;h=h->next){
		if(strcmp(eventName,h->eventName) != 0){continue;}
		h->handler(w);
	}
}

static void widgetDrawButton(widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color       = 0xFF555555;
	u32 tcolor      = 0xFF777777;
	u32 bcolor      = 0xFF333333;
	int textYOff    = (h - (2*8))/2;
	int textXOff    = (w-(strnlen(wid->label,w/16)*16))/2;

	if(wid == widgetFocused){
		 color = 0xFFAA6666;
		tcolor = 0xFFCC8888;
		bcolor = 0xFF884444;
	}else if(wid->flags & WIDGET_CLICKED){
		color = 0xFF2A2A2A;
		textXOff+=1;
		textYOff+=1;
		int tmp = tcolor;
		tcolor = bcolor;
		bcolor = tmp;
	}else if(wid->flags & WIDGET_HOVER){
		color = 0xFF444444;
	}

	textMeshSolidBox(m,x+1, y+1,w-1,h-1, color);
	textMeshSolidBox(m,x+1, y  ,w-2,  1,tcolor);
	textMeshSolidBox(m,x  , y+1,  1,h-2,tcolor);
	textMeshSolidBox(m,x+1, y+h,w-2,  1,bcolor);
	textMeshSolidBox(m,x+w, y+1,  1,h-2,bcolor);

	textMeshAddLinePS(m,x+textXOff,y+textYOff,2,wid->label);
}

static void widgetDrawButtondel(widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color       = 0xFF555555;
	u32 tcolor      = 0xFF777777;
	u32 bcolor      = 0xFF333333;

	u32 dcolor      = 0xFF555599;
	u32 dtcolor     = 0xFF7777AA;
	u32 dbcolor     = 0xFF333377;

	int textYOff    = (h - (2*8))/2;
	int textXOff    = (w-(strnlen(wid->label,w/16)*16))/2;

	if(wid == widgetFocused){
		 color = 0xFFAA6666;
		tcolor = 0xFFCC8888;
		bcolor = 0xFF884444;
	}else{
		if((int)mousex > (x+w-40)){
			if(wid->flags & WIDGET_CLICKED){
				dcolor  = 0xFF2A2A6A;
				int tmp = dtcolor;
				dtcolor = dbcolor;
				dbcolor = tmp;
			}else if(wid->flags & WIDGET_HOVER){
				dcolor  = 0xFF444488;
			}
		}else{
			if(wid->flags & WIDGET_CLICKED){
				color = 0xFF2A2A2A;
				textXOff+=1;
				textYOff+=1;
				int tmp = tcolor;
				tcolor = bcolor;
				bcolor = tmp;
			}else if(wid->flags & WIDGET_HOVER){
				color = 0xFF444444;
			}
		}
	}

	textMeshSolidBox(m,x+1, y+1,w-41,h-1,  color);
	textMeshSolidBox(m,x+1, y  ,w-42,  1, tcolor);
	textMeshSolidBox(m,x  , y+1,   1,h-2, tcolor);
	textMeshSolidBox(m,x+1, y+h,w-42,  1, bcolor);

	textMeshSolidBox(m,x+w-41, y+1  ,40,h-1,  dcolor);
	textMeshSolidBox(m,x+w   , y+1  , 1,h-2, dbcolor);
	textMeshSolidBox(m,x+w-41, y+h-1,40,  1, dbcolor);
	textMeshSolidBox(m,x+w-41, y+1  ,40,  1, dtcolor);

	textMeshAddLinePS(m,x+textXOff,y+textYOff,2,wid->label);
	textMeshAddLinePS(m,x+w-24,y+textYOff,2,"X");
}

static void widgetDrawBackground(widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	float u = 19.f/32.f*128.f;
	float v = 31.f/32.f*128.f;
	float s =  1.f/32.f*128.f;

	textMeshAddVert(m,x,y,u  ,v  ,0xFFFFAF63);
	textMeshAddVert(m,x,h,u  ,v+s,0xFFFF6825);
	textMeshAddVert(m,w,h,u+s,v+s,0xFFFF6825);

	textMeshAddVert(m,w,h,u+s,v+s,0xFFFF6825);
	textMeshAddVert(m,w,y,u+s,v  ,0xFFFFAF63);
	textMeshAddVert(m,x,y,u  ,v  ,0xFFFFAF63);
}

static void widgetDrawPanel(widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	textMeshSolidBox(m,x,y,w,h,0xD0303030);
}

static void widgetCheckEvents(widget *wid, int x, int y, int w, int h){
	if(mouseInBox(x,y,w,h)){
		wid->flags |= WIDGET_HOVER;
		if(mouseClicked[0]){
			wid->flags |= WIDGET_CLICKED;
		}else{
			if(wid->flags & WIDGET_CLICKED){
				if((wid->type == WIDGET_BUTTONDEL) && ((int)mousex > x+w-40)){
					widgetEmit(wid,"altclick");
				}else{
					widgetFocus(wid);
					widgetEmit(wid,"click");
				}
			}
			wid->flags &= ~WIDGET_CLICKED;
		}
	}else{
		wid->flags &= ~(WIDGET_HOVER | WIDGET_CLICKED);
	}
}

static void widgetAnimate(widget *wid, int x, int y, int w, int h){
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

	if(wid->flags & WIDGET_ANIMATE){return;}
	int wx = wid->x + x;
	int wy = wid->y + y;
	int ww = wid->w;
	int wh = wid->h;

	if(wid->x < 0){
		wx = (x + w) - wid->w + (wid->x+1);
	}
	if(wid->y < 0){
		wy = (y + h) - wid->h + (wid->y+1);
	}
	if(ww < 0){ ww = w+(wid->w+1); }
	if(wh < 0){ wh = h+(wid->h+1); }

	if((ww == 0) || (wh == 0)){
		wid->flags |= WIDGET_HIDDEN;
		return;
	}

	if((wx >= screenWidth)  ||
	   (wy >= screenHeight) ||
	   (wx+ww <= 0)         ||
	   (wy+wh <= 0)){
		wid->flags |= WIDGET_HIDDEN;
	}
}

static void widgetDrawTextInput(widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color    = 0xFF333333;
	u32 bcolor   = 0xFF555555;
	u32 tcolor   = 0xFF222222;
	int textYOff = (h - (2*8))/2;
	int textXOff = 8;
	int size     = 2;

	if((wid->flags & WIDGET_BIGGER) == WIDGET_BIGGER){
		size = 8;
	}else if(wid->flags & WIDGET_BIG){
		size = 4;
	}else if(wid->flags & WIDGET_SMALL){
		size = 1;
	}

	if(widgetFocused == wid){
		color = 0xFF292929;
	}

	textMeshSolidBox(m,x+1, y+1,w-1,h-1, color);
	textMeshSolidBox(m,x+1, y  ,w-2,  1,tcolor);
	textMeshSolidBox(m,x  , y+1,  1,h-2,tcolor);
	textMeshSolidBox(m,x+1, y+h,w-2,  1,bcolor);
	textMeshSolidBox(m,x+w, y+1,  1,h-2,bcolor);

	if(wid->vals == NULL){return;}
	if(wid->vals[0] == 0){
		textMeshAddLinePS(m,x+textXOff,y+textYOff,size,wid->label);
	}else{
		textMeshAddLinePS(m,x+textXOff,y+textYOff,size,wid->vals);
	}
	if((widgetFocused == wid) && (getTicks() & 512)){
		textMeshAddGlyph(m, x+textXOff+(textInputCursorPos*size*8), y+textYOff, size, 127);
	}
}

static void widgetDrawLabel(widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)w;
	(void)h;
	int size = 2;

	if((wid->flags & WIDGET_BIGGER) == WIDGET_BIGGER){
		size = 8;
	}else if(wid->flags & WIDGET_BIG){
		size = 4;
	}else if(wid->flags & WIDGET_SMALL){
		size = 1;
	}

	m->sx = x;
	if(wid->label != NULL){
		textMeshAddLinePS(m,x,y,size,wid->label);
	}
	if(wid->vals != NULL){
		textMeshAddLinePS(m,m->sx,y,size,wid->vals);
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
	widgetAnimate(wid,x,y,w,h);

	switch(wid->type){
		case WIDGET_PANEL:
			widgetDrawPanel (wid,m,x,y,w,h);
			break;
		case WIDGET_BUTTON:
			widgetDrawButton(wid,m,x,y,w,h);
			break;
		case WIDGET_BACKGROUND:
			widgetDrawBackground(wid,m,x,y,w,h);
			break;
		case WIDGET_LABEL:
			widgetDrawLabel(wid,m,x,y,w,h);
			break;
		case WIDGET_BUTTONDEL:
			widgetDrawButtondel(wid,m,x,y,w,h);
			break;
		case WIDGET_TEXTINPUT:
			widgetDrawTextInput(wid,m,x,y,w,h);
			break;
	}

	for(widget *c=wid->child;c!=NULL;c=c->next){
		widgetDraw(c,m,x,y,w,h);
	}
}

void widgetSlideW(widget *w, int nw){
	if(nw == 0){
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEW | WIDGET_NOSELECT;
		w->gw     =   0;
	}else{
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEW;
		w->gw = nw;
	}
}

void widgetSlideH(widget *w, int nh){
	if(nh == 0){
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEH | WIDGET_NOSELECT;
		w->gh     =   0;
	}else{
		w->flags &= ~(WIDGET_NOSELECT | WIDGET_HIDDEN);
		w->flags |= WIDGET_ANIMATEH;
		w->gh = nh;
	}
}
