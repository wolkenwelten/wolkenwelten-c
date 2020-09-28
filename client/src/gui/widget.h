#pragma once
#include "../../../common/src/common.h"

typedef struct widget widget;
typedef struct eventHandler eventHandler;

struct eventHandler {
	const char *eventName;
	void (*handler)(widget *);
	eventHandler *next;
};

struct widget {
	int x,y,w,h;
	uint type,flags;
	int gx,gy,gw,gh;

	widget *parent,*child;
	widget *prev,*next;
	eventHandler  *firstHandler;

	const char *label;
	union {
		char *vals;
		int   vali;
		uint  valu;
	};
};
#define WIDGET_PANEL      1
#define WIDGET_BUTTON     2
#define WIDGET_SPACE      3
#define WIDGET_BACKGROUND 4
#define WIDGET_LABEL      5
#define WIDGET_BUTTONDEL  6
#define WIDGET_TEXTINPUT  7

#define WIDGET_HIDDEN   (1   )
#define WIDGET_HOVER    (1<<1)
#define WIDGET_CLICKED  (1<<2)
#define WIDGET_NOSELECT (1<<3)
#define WIDGET_ANIMATEX (1<<4)
#define WIDGET_ANIMATEY (1<<5)
#define WIDGET_ANIMATEW (1<<6)
#define WIDGET_ANIMATEH (1<<7)
#define WIDGET_SMALL    (1<<8)
#define WIDGET_BIG      (1<<9)
#define WIDGET_BIGGER   (WIDGET_SMALL | WIDGET_BIG)

#define WIDGET_ANIMATE (15<<4)
#define WIDGET_HNS      (WIDGET_HIDDEN | WIDGET_NOSELECT)

extern widget *widgetFocused;

widget *widgetNew     (int type);
widget *widgetNewC    (int type, widget *p);

widget *widgetNewCP   (int type, widget *p, int x, int y, int w, int h);
widget *widgetNewCPL  (int type, widget *p, int x, int y, int w, int h, const char *label);
widget *widgetNewCPLH (int type, widget *p, int x, int y, int w, int h, const char *label,const char *eventName, void (*handler)(widget *));
void    widgetFree    (widget *w);
void    widgetFocus   (widget *w);
void    widgetEmpty   (widget *w);
void    widgetChild   (widget *parent, widget *child);
void    widgetChildPre(widget *parent, widget *child);
widget *widgetNextSel (widget *cur);
widget *widgetPrevSel (widget *cur);
void    widgetBind    (widget *w, const char *eventName, void (*handler)(widget *));
void    widgetEmit    (widget *w, const char *eventName);
void    widgetDraw    (widget *w, textMesh *mesh, int x, int y, int pw, int ph);
void    widgetLayVert (widget *w, int padding);
void    widgetSlideW  (widget *w, int nw);
void    widgetSlideH  (widget *w, int nh);

void drawButton(textMesh *m, const char *label, int state, int x, int y, int w, int h);
bool mouseInBox(uint x, uint y, uint w, uint h);
