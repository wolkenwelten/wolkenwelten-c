#pragma once
#include "../../../common/src/common.h"

#include "../misc/lisp.h"

typedef struct widget widget;
typedef struct eventHandler eventHandler;

struct eventHandler {
	const char *eventName;
	bool lisp;
	union {
		void (*handler)(widget *);
		lVal *lispHandler;
	};
	eventHandler *next;
};

typedef enum {
	wNone = 0,
	wSpace,
	wPanel,
	wBackground,
	wHR,
	wLabel,
	wTextInput,
	wButton,
	wRadioButton,
	wButtonDel,
	wSlider,
	wItemSlot,
	wRecipeSlot,
	wRecipeInfo,
	wGameScreen,
	wTextScroller,
	wTextLog
} widgetType;

struct widget {
	int x,y,w,h;
	widgetType type;
	uint flags;
	int gx,gy,gw,gh;

	widget *parent,*child;
	widget *prev,*next;
	eventHandler *firstHandler;

	const char *label;
	union {
		char  *vals;
		char **valss;
		int    vali;
		uint   valu;
		item  *valItem;
	};
};

#define WIDGET_HIDDEN      (1    )
#define WIDGET_HOVER       (1<< 1)
#define WIDGET_CLICKED     (1<< 2)
#define WIDGET_NOSELECT    (1<< 3)
#define WIDGET_ANIMATEX    (1<< 4)
#define WIDGET_ANIMATEY    (1<< 5)
#define WIDGET_ANIMATEW    (1<< 6)
#define WIDGET_ANIMATEH    (1<< 7)
#define WIDGET_SMALL       (1<< 8)
#define WIDGET_BIG         (1<< 9)
#define WIDGET_BIGGER      (WIDGET_SMALL | WIDGET_BIG)
#define WIDGET_ALT_CLICKED (1<<10)
#define WIDGET_MID_CLICKED (1<<11)
#define WIDGET_ACTIVE      (1<<12)
#define WIDGET_LISP        (1<<13)

#define WIDGET_ANIMATE (15<<4)
#define WIDGET_HNS      (WIDGET_HIDDEN | WIDGET_NOSELECT)

extern widget *widgetFocused;



widget *widgetNew     (widgetType type);
widget *widgetNewC    (widgetType type, widget *p);
widget *widgetNewCP   (widgetType type, widget *p, int x, int y, int w, int h);
widget *widgetNewCPL  (widgetType type, widget *p, int x, int y, int w, int h, const char *label);
widget *widgetNewCPLH (widgetType type, widget *p, int x, int y, int w, int h, const char *label,const char *eventName, void (*handler)(widget *));
widget *widgetGet     (uint i);
void    widgetFree    (widget *w);
void    widgetExport  (widget *w, const char *symbol);
void    widgetAddLispFunctions(lClosure *c);

void    widgetFocus   (widget *w);
void    widgetEmpty   (widget *w);
void    widgetChild   (widget *parent, widget *child);
void    widgetChildPre(widget *parent, widget *child);
widget *widgetNextSel (const widget *cur);
widget *widgetPrevSel (const widget *cur);
void    widgetBind    (widget *w, const char *eventName, void (*handler)(widget *));
void    widgetBindL   (widget *w, const char *eventName, lVal *val);
int     widgetEmit    (widget *w, const char *eventName);
void    widgetDraw    (widget *w, textMesh *mesh, int x, int y, int pw, int ph);
void    widgetLayVert (widget *w, int padding);
void    widgetLabel   (widget *w, const char *newLabel);
void    widgetFinish  (widget *w);
void    widgetSlideW  (widget *w, int nw);
void    widgetSlideH  (widget *w, int nh);
void    widgetSlideX  (widget *w, int nw);
void    widgetSlideY  (widget *w, int nh);
void    widgetAddEntry(widget *w, const char *entry);
