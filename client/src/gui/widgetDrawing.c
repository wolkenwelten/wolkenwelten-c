#define _GNU_SOURCE
#include "widgetDrawing.h"

#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../gfx/textMesh.h"
#include "../sdl/sdl.h"

#include <math.h>
#include <string.h>

static void widgetDrawButton(const widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color    = 0xFF555555;
	u32 tcolor   = 0xFF777777;
	u32 bcolor   = 0xFF333333;
	int textYOff = (h - (2*8))/2;
	int textXOff = (w-(strnlen(wid->label,w/16)*16))/2;

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

	textMeshVGradient(m,x+1,y+1,w-2,h-2, color,bcolor);
	textMeshSolidBox (m,x+1,y  ,w-2,  1,tcolor);
	textMeshSolidBox (m,x  ,y+1,  1,h-2,tcolor);
	textMeshSolidBox (m,x+1,y+h-1,w-2,  1,bcolor);
	textMeshSolidBox (m,x+w-1,y+1,  1,h-2,bcolor);

	textMeshAddLinePS(m,x+textXOff,y+textYOff,2,wid->label);
}

static void widgetDrawButtondel(const widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color    = 0xFF555555;
	u32 tcolor   = 0xFF777777;
	u32 bcolor   = 0xFF333333;

	u32 dcolor   = 0xFF555599;
	u32 dtcolor  = 0xFF7777AA;
	u32 dbcolor  = 0xFF333377;

	int textYOff = (h - (2*8))/2;
	int textXOff = (w-(strnlen(wid->label,w/16)*16))/2;

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

	textMeshVGradient(m,x+1, y+1,w-41,h-1, color,bcolor);
	textMeshSolidBox (m,x+1, y  ,w-42,  1, tcolor);
	textMeshSolidBox (m,x  , y+1,   1,h-2, tcolor);
	textMeshSolidBox (m,x+1, y+h-1,w-42,  1, bcolor);

	textMeshVGradient(m,x+w-41, y+1,40,h-2, dcolor,dbcolor);
	textMeshSolidBox (m,x+w-1 , y+1  , 1,h-2, dbcolor);
	textMeshSolidBox (m,x+w-41, y+h-1,40,  1, dbcolor);
	textMeshSolidBox (m,x+w-41, y  ,40,  1, dtcolor);

	textMeshAddLinePS(m,x+textXOff,y+textYOff,2,wid->label);
	textMeshAddLinePS(m,x+w-24,y+textYOff,2,"X");
}

static void widgetDrawBackground(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	static int i = 0;
	int o = h/2 + sin(++i/256.f)*((float)(h/3));
	textMeshVGradient(m,x,y    ,w,o,0xFFFFBF83, 0xFFFF6825);
	textMeshVGradient(m,x,y+o,w,h-o,0xFFFF6825, 0xFFE82410);
}

static void widgetDrawPanel(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	textMeshSolidBox(m,x,y,w,h,0xD0303030);
}

static void widgetDrawTextInput(const widget *wid, textMesh *m, int x, int y, int w, int h){
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

	textMeshSolidBox(m,x+1, y+1,w-1,h-2, color);
	textMeshSolidBox(m,x+1, y  ,w-2,  1,tcolor);
	textMeshSolidBox(m,x  , y+1,  1,h-2,tcolor);
	textMeshSolidBox(m,x+1, y+h-1,w-2,  1,bcolor);
	textMeshSolidBox(m,x+w-1, y+1,  1,h-2,bcolor);

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

static void widgetDrawLabel(const widget *wid, textMesh *m, int x, int y, int w, int h){
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

static void widgetDrawSlider(const widget *wid, textMesh *m, int x, int y, int w, int h){
	u32  bcolor = 0xFF555555;
	u32  tcolor = 0xFF222222;
	u32 abcolor = 0xFFC08840;
	u32 atcolor = 0xFFA04123;
	const int textYOff = (h - (2*8))/2;
	const int textXOff = (w-(strnlen(wid->label,w/16)*16))/2;
	const int size     = 2;
	const float v  = MAX(0,MIN(1,wid->vali / 4096.f));
	int o = v*(w-2);

	textMeshVGradient(m,x+1, y+1,o-3,h-2, abcolor,atcolor);
	textMeshSolidBox(m,x+o+3, y+1,w-o-3,h-2,tcolor);

	if(wid == widgetFocused){
		tcolor = abcolor;
		bcolor = atcolor;
	}
	textMeshSolidBox(m,x+1  , y    ,w-2,  1,tcolor);
	textMeshSolidBox(m,x    , y+1  ,  1,h-2,tcolor);
	textMeshSolidBox(m,x+1  , y+h-1,w-2,  1,bcolor);
	textMeshSolidBox(m,x+w-1, y+1  ,  1,h-2,bcolor);

	bcolor = 0xFF555555;
	tcolor = 0xFF222222;

	textMeshVGradient(m,x+o-2, y+1,4,h-2, bcolor,tcolor);
	textMeshSolidBox (m,x+o-3, y+1,1,h-2,tcolor);
	textMeshSolidBox (m,x+o+2, y+1,1,h-2,tcolor);
	textMeshSolidBox (m,x+o-2, y    ,4,1,tcolor);
	textMeshSolidBox (m,x+o-2, y+h-1,4,1,tcolor);

	m->sx = x;
	if(wid->label != NULL){
		textMeshAddLinePS(m,x+textXOff,y+textYOff,size,wid->label);
	}
}

static void widgetDrawHR(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	const u32 bcolor = 0xCC444444;
	const u32 tcolor = 0xCC111111;
	const uint o = MAX(0,(h/2)-2);
	textMeshVGradient(m,x,y+o,w,4,tcolor,bcolor);
}


void widgetDrawSingle(const widget *wid, textMesh *m,int x, int y, int w, int h){
	if(wid == NULL){return;}

	switch(wid->type){
		case WIDGET_PANEL:
			widgetDrawPanel(wid,m,x,y,w,h);
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
		case WIDGET_SLIDER:
			widgetDrawSlider(wid,m,x,y,w,h);
			break;
		case WIDGET_HR:
			widgetDrawHR(wid,m,x,y,w,h);
			break;
	}
}
