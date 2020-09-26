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

	widget *parent;
	widget *sibling;
	widget *child;
	eventHandler  *firstHandler;

	char *label;
	union {
		char *vals;
		int   vali;
		uint  valu;
	};
};
#define WIDGET_PANEL   1
#define WIDGET_BUTTON  2
#define WIDGET_SPACE   3

#define WIDGET_HIDDEN  (1   )
#define WIDGET_HOVER   (1<<1)
#define WIDGET_CLICKED (1<<2)

widget *widgetNew     (int type);
widget *widgetNewC    (int type, widget *p);
widget *widgetNewCP   (int type, widget *p, int x, int y, int w, int h);
widget *widgetNewCPL  (int type, widget *p, int x, int y, int w, int h, char *label);
widget *widgetNewCPLH (int type, widget *p, int x, int y, int w, int h, char *label,const char *eventName, void (*handler)(widget *));
void    widgetFree    (widget *w);
void    widgetEmpty   (widget *w);
void    widgetChild   (widget *parent, widget *child);
void    widgetChildPre(widget *parent, widget *child);
void    widgetBind    (widget *w, const char *eventName, void (*handler)(widget *));
void    widgetEmit    (widget *w, const char *eventName);
void    widgetDraw    (widget *w, textMesh *mesh, int x, int y, int pw, int ph);
void    widgetLayVert (widget *w, int padding);

void drawButton(textMesh *m, const char *label, int state, int x, int y, int w, int h);
bool mouseInBox(uint x, uint y, uint w, uint h);
