#define _DEFAULT_SOURCE
#include "widget.h"

#include "../gui/gui.h"
#include "../gfx/textMesh.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


widget *widgetNew(int type){
	widget *wid = calloc(1,sizeof(widget));
	if(wid == NULL){return NULL;}
	wid->type = type;
	return wid;
}
widget *widgetNewP  (int type, int x, int y, int w, int h){
	widget *wid = widgetNew(type);
	if(wid == NULL){return NULL;}
	wid->x = x;
	wid->y = y;
	wid->w = w;
	wid->h = h;
	return wid;
}
widget *widgetNewPL (int type, int x, int y, int w, int h, char *label){
	widget *wid = widgetNewP(type,x,y,w,h);
	if(wid == NULL){return NULL;}
	wid->label = label;
	return wid;
}
widget *widgetNewPLH(int type, int x, int y, int w, int h, char *label,const char *eventName, void (*handler)(widget *)){
	widget *wid = widgetNewPL(type,x,y,w,h,label);
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
			w->parent->child = w->sibling;
		}else{
			// ToDo
		}
	}
	free(w);
}

void widgetEmpty(widget *w){
	widget *n=NULL;
	if(w == NULL){return;}
	for(widget *c=w->child;c!=NULL;c=n){
		n = c->sibling;
		widgetFree(c);
	}
	w->child = NULL;
}

void widgetChild(widget *parent, widget *child){
	if(child->parent != NULL){
		child->parent->child = child->sibling;
		child->sibling = NULL;
	}
	if(parent->child != NULL){
		if(child->sibling != NULL){
			fprintf(stderr,"Widget already has a sibling!\n");
		}
		widget *c;
		for(c = parent->child;c->sibling != NULL;c = c->sibling){}
		c->sibling = child;
	}else{
		parent->child = child;
		child->parent = parent;
	}
}


void widgetChildPre(widget *parent, widget *child){
	if(child->parent != NULL){
		child->parent->child = child->sibling;
		child->sibling = NULL;
	}
	if(parent->child != NULL){
		if(child->sibling != NULL){
			fprintf(stderr,"Widget already has a sibling!\n");
		}
		child->sibling = parent->child;
	}
	parent->child = child;
	child->parent = parent;
}

void widgetLayVert(widget *w, int padding){
	int y = padding;
	for(widget *c=w->child;c!=NULL;c=c->sibling){
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
		if(strcmp(eventName,h->eventName) == 0){
			h->handler(w);
		}
	}
}

static void widgetDrawButton(widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color       = 0xFF555555;
	u32 tcolor      = 0xFF777777;
	u32 bcolor      = 0xFF333333;
	int textYOff    = (h - (2*8))/2;
	int textXOff    = (w-(strnlen(wid->label,w/16)*16))/2;

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

	textMeshSolidBox(m,x+1, y+1,w-1,h-1, color);
	textMeshSolidBox(m,x  , y  ,w  ,  1,tcolor);
	textMeshSolidBox(m,x  , y  ,1  ,  h,tcolor);
	textMeshSolidBox(m,x  , y+h,w  ,  1,bcolor);
	textMeshSolidBox(m,x+w, y  ,1  ,  h,bcolor);

	textMeshAddStrPS(m,x+textXOff,y+textYOff,2,wid->label);
}

static void widgetDrawPanel(widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	(void)m;
	(void)x;
	(void)y;
	(void)w;
	(void)h;
}

static void widgetCheckEvents(widget *wid, int x, int y, int w, int h){
	if(mouseInBox(x,y,w,h)){
		wid->flags |= WIDGET_HOVER;
		if(mouseClicked[0]){
			wid->flags |= WIDGET_CLICKED;
		}else{
			if(wid->flags & WIDGET_CLICKED){
				widgetEmit(wid,"click");
			}
			wid->flags &= ~WIDGET_CLICKED;
		}
	}else{
		wid->flags &= ~(WIDGET_HOVER | WIDGET_CLICKED);
	}
}

void widgetDraw(widget *wid, textMesh *m,int px, int py, int pw, int ph){
	if(wid == NULL){return;}
	if(wid->flags & WIDGET_HIDDEN){return;}
	int x = px + wid->x;
	int y = py + wid->y;
	int w = wid->w;
	int h = wid->h;
	if(x < 0){
		x = (px + pw) - wid->w + (wid->x+1);
	}
	if(y < 0){
		y = (py + ph) - wid->h + (wid->y+1);
	}
	if(w < 0){ w = pw+(wid->w+1); }
	if(h < 0){ h = ph+(wid->w+1); }
	widgetCheckEvents(wid,x,y,w,h);

	switch(wid->type){
		case WIDGET_PANEL:
			widgetDrawPanel (wid,m,x,y,w,h);
			break;
		case WIDGET_BUTTON:
			widgetDrawButton(wid,m,x,y,w,h);
			break;
	}
	for(widget *c=wid->child;c!=NULL;c=c->sibling){
		widgetDraw(c,m,x,y,w,h);
	}
}

void drawButton(textMesh *m, const char *label, int state, int x, int y, int w, int h){
	u32 color    = 0xFF555555;
	u32 tcolor   = 0xFF777777;
	u32 bcolor   = 0xFF333333;
	int textYOff = (h - (2*8))/2;
	int textXOff = (w-(strnlen(label,w/16)*16))/2;

	if(state == 0){
		if(mouseInBox(x,y,w,h)){
			if(mouseClicked[0]){
				state = 2;
			}else{
				state = 1;
			}
		}
	}

	if(state == 1){
		color = 0xFF444444;
	}else if(state == 2){
		color = 0xFF2A2A2A;
		textXOff+=1;
		textYOff+=1;
		int tmp = tcolor;
		tcolor = bcolor;
		bcolor = tmp;
	}

	textMeshSolidBox(m,x+1, y+1,w-1,h-1,color);
	textMeshSolidBox(m,x  , y  ,w, 1,tcolor);
	textMeshSolidBox(m,x  , y  ,1, h,tcolor);
	textMeshSolidBox(m,x  , y+h,w, 1,bcolor);
	textMeshSolidBox(m,x+w, y  ,1, h,bcolor);

	textMeshAddStrPS(m,x+textXOff,y+textYOff,2,label);
}


bool mouseInBox(uint x, uint y, uint w, uint h){
	if(mousex < x  ){return false;}
	if(mousex > x+w){return false;}
	if(mousey < y  ){return false;}
	if(mousey > y+h){return false;}
	return true;
}
