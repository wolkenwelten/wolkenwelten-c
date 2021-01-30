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

#include "widgetDrawing.h"

#include "../game/character.h"
#include "../game/recipe.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../gfx/textMesh.h"
#include "../sdl/sdl.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/game/item.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

static void widgetDrawLispLine(textMesh *m, int x, int y, int size, int w, int h, const char *rawLine, int lambda, int mark, int cursor){
	(void)h;
	int openParens = 0, cx = x, cy = y, oldFont = m->font;;
	u32 cfgc = 0xFFFFFFFF;
	u8 c;
	m->font = 1;

	int selMin = MIN(mark,cursor),selMax = MAX(mark,cursor);
	if(selMin < 0){selMax = -1;}

	static u32 colors[16] = {
		0x00000000,
		0x60FF8040,
		0x608040FF,
		0x6040FF80,
		0x6040F0F0,
		0x6080FF40,
		0x604080FF,
		0x60FF40FF,

		0x60FFFFFF,
		0xA0FF8040,
		0xA08040FF,
		0xA040FF80,
		0xA040F0F0,
		0xA080FF40,
		0xA04080FF,
		0xA0FF40FF
	};
	if(lambda){
		if(lambda == 2){
			textMeshVGradient(m, cx-size*4, y-size*4, size*8*3 ,size*8*2,0xA01030FF,0x800410E0);
			textMeshVGradient(m, cx-size*4-2+size*8*3, y-size*4, 2 ,size*8*2,0x800410E0,0xA01030FF);
		}
		textMeshAddGlyph(m, cx, y, size, 20, colors[1] | 0xFF000000, 0x00000000);
		cx += size*8;
		textMeshAddGlyph(m, cx, y, size, '>', colors[1] | 0xFF000000, 0x00000000);
		cx += size*8;
		cx += size*8;
	}
	int selActive = 0;
	for(const char *line = rawLine;*line != 0;line++){
		const int curPos = line - rawLine;
		if(curPos >= selMax){
			selActive = 0;
		}else if((curPos >= selMin) && (curPos < selMax)){
			selActive = 8;
		}
		if(cx > w){break;}
		if(((u8)line[0] == 0xCE) && ((u8)line[1] == 0xBB)){ // UTF-8 Lambda
			line++;
			c = 20;
		}else if((u8)line[0] == '\n'){
			cy += size * 8;
			cx = x;
			continue;
		}else if(*line == '\033'){
			int fgc = -1, bgc = -1;;
			line += parseAnsiCode(line,&fgc,&bgc) - 1;
			if(fgc >= 0){cfgc = colorPalette[fgc];}
			//if(bgc >= 0){m->bgc = colorPalette[bgc];}
			continue;
		}else{
			c = *line;
		}
		if(*line == '('){
			openParens++;
			textMeshAddGlyphHG(m, cx, cy, size, c, cfgc, colors[((openParens-1)&7) | selActive],colors[(openParens&7) | selActive]);
		}else if(*line == ')'){
			textMeshAddGlyphHG(m, cx, cy, size, c, cfgc, colors[((openParens)&7) | selActive],colors[((openParens-1)&7) | selActive]);
			openParens--;
		}else{
			textMeshAddGlyph(m, cx, cy, size, c, cfgc, colors[(openParens&7) | selActive]);
		}
		cx += size * 8;
	}
	for(;openParens > 0;openParens--){
		if(cx > w){break;}
		textMeshAddGlyphHG(m, cx, cy, size, ')', cfgc & 0x7FFFFFFF, colors[(openParens)&7] >> 1,colors[(openParens-1)&7] >> 1);
		cx += size * 8;
	}
	m->font = oldFont;
}

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
	}
	if(wid->flags & WIDGET_CLICKED){
		if(wid != widgetFocused){
			color = 0xFF2A2A2A;
		}
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

	textMeshAddStrPS(m,x+textXOff,y+textYOff,2,wid->label);
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

	textMeshAddStrPS(m,x+textXOff,y+textYOff,2,wid->label);
	textMeshAddStrPS(m,x+w-24,y+textYOff,2,"X");
}

static void widgetDrawRadioButton(const widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color    = 0xFF555555;
	u32 tcolor   = 0xFF777777;
	u32 bcolor   = 0xFF333333;
	int textYOff = (h - (2*8))/2;
	int textXOff = (w-(strnlen(wid->label,w/16)*16))/2;

	if(wid == widgetFocused){
		 color = 0xFFAA6666;
		tcolor = 0xFFCC8888;
		bcolor = 0xFF884444;
	}
	if((wid->flags & WIDGET_CLICKED) || (wid->flags & WIDGET_ACTIVE)){
		if(wid != widgetFocused){
			color = 0xFF2A2A2A;
		}
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

	textMeshAddStrPS(m,x+textXOff,y+textYOff,2,wid->label);
}

static void widgetDrawBackground(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	int o = h/2 + sin(getTicks()*0.001f)*((float)(h/6));
	textMeshVGradient(m,x,y    ,w,o,0xFFFFBF83, 0xFFFF6825);
	textMeshVGradient(m,x,y+o,w,h-o,0xFFFF6825, 0xFFE82410);
}

static void widgetDrawPanel(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	textMeshSolidBox(m,x,y,w,h,0xE0303030);
}

static void widgetDrawTextInput(const widget *wid, textMesh *m, int x, int y, int w, int h){
	u32 color    = 0xFF333333;
	u32 bcolor   = 0xFF555555;
	u32 tcolor   = 0xFF222222;
	int textYOff = (h - (2*8))/2;
	int textXOff = 8;
	int size     = 2;
	bool isLisp  = (wid->flags & WIDGET_LISP_SYNTAX_HIGHLIGHT) != 0;

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
	int oldmx = m->mx;
	m->mx     = x+w - size*8;
	if(isLisp){
		if(widgetFocused == wid){
			widgetDrawLispLine(m, x+textXOff, y+textYOff, size, w-textXOff, size*8, wid->vals, 2, textInputMark, textInputCursorPos);
		}else{
			widgetDrawLispLine(m, x+textXOff, y+textYOff, size, w-textXOff, size*8, wid->vals, 2, -1, -1);
		}
	}else{
		if(wid->vals[0] == 0){
			u32 fgc = m->fgc;
			m->fgc &= 0x7FFFFFFF;
			textMeshAddStrPS(m,x+textXOff,y+textYOff,size,wid->label);
			m->fgc = fgc;
		}else{
			textMeshAddStrPS(m,x+textXOff,y+textYOff,size,wid->vals);
		}
	}

	if(widgetFocused == wid){
		uint alpha = ((getTicks() >> 1) & 511);
		if(alpha > 255){alpha = 512 - alpha;}
		int cx = x+textXOff+(textInputCursorPos*size*8);
		if(isLisp){cx += size*8*3;}
		if(cx < m->mx){
			textMeshAddGlyph(m, cx, y+textYOff, size, 127,0xFFFFFF | (alpha << 24),0x00000000);
		}
		if((!isLisp) && (textInputMark >= 0)){
			const int sMin = MIN(textInputCursorPos,textInputMark);
			const int sMax = MAX(textInputCursorPos,textInputMark);
			const int sx = sMin*size*8;
			const int sw = (sMax*size*8)-sx;
			textMeshSolidBox (m,x+textXOff+sx,y+textYOff,sw,size*8, 0x40FFFFFF);
		}
	}
	m->mx = oldmx;
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
		textMeshAddStrPS(m,x,y,size,wid->label);
	}
	if(wid->vals != NULL){
		textMeshAddStrPS(m,m->sx,y,size,wid->vals);
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
		textMeshAddStrPS(m,x+textXOff,y+textYOff,size,wid->label);
	}
}

static void widgetDrawHR(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)wid;
	const u32 bcolor = 0xCC444444;
	const u32 tcolor = 0xCC111111;
	const uint o = MAX(0,(h/2)-2);
	textMeshVGradient(m,x,y+o,w,4,tcolor,bcolor);
}

static void widgetDrawItemSlot(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)h;
	int style = 0;
	item *itm = wid->valItem;
	if((wid == widgetFocused) || (wid->flags & WIDGET_HOVER) || (itm == characterGetActiveItem(player))){
		style = 1;
	}
	textMeshItem(m, x, y, w, style, itm);
}

static void widgetDrawRecipeSlot(const widget *wid, textMesh *m, int x, int y, int w, int h){
	int style = 0;
	const item recipe = recipeGetResult(wid->valu);
	i16 a = recipeCanCraft(player,wid->valu);
	if(a == 0){
		style = 2;
	}
	if((wid == widgetFocused) || (wid->flags & WIDGET_HOVER)){
		style = 1;
	}
	textMeshItemSlot(m,x,y,MIN(w,h),style,recipe.ID,a);
}

static void widgetDrawRecipeInfo(const widget *wid, textMesh *m, int x, int y, int w, int h){
	const uint ticks = getTicks()>>4;
	const uint ts    = MIN(w,h);
	uint ii,xx,r = wid->vali;
	if(r >= recipeGetCount()){return;}
	const int animX = sin((float)ticks/24.f)*ts/8;
	const int animY = cos((float)ticks/24.f)*ts/8;
	const item result = recipeGetResult(r);


	for(ii=0;ii<4;ii++){
		xx = ii*2*ts + x;
		item ingred = recipeGetIngredient(r,ii);
		if(itemIsEmpty(&ingred)){break;}
		ingred.ID = ingredientSubstituteGetSub(ingred.ID,(ticks/96) % (ingredientSubstituteGetAmount(ingred.ID)+1));

		if(ii > 0){
			textMeshBox(m,xx-ts+ts/4+animX,y+ts/4+animY,ts/2,ts/2,24.f/32,31.f/32.f,1.f/32.f,1.f/32.f,~1);
		}
		textMeshItem(m,xx,y,ts,3,&ingred);
	}

	xx = ii*2*ts + x;
	textMeshBox(m,xx-ts+ts/4+animX*2,y+ts/4,ts/2,ts/2,25.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
	textMeshItemSlot(m,xx,y,ts,3,result.ID,result.amount);
}

static void widgetDrawTextScroller(const widget *wid, textMesh *m, int x, int y, int w, int h){
	static int oy = -128;
	static int ov = 2;
	(void)w;
	(void)h;

	m->wrap = 1;
	bool overflow = textMeshPrintfPS(m,x+16,y+16-oy,2,"%s",wid->label);
	m->wrap = 0;

	if(!overflow && (abs(oy) < ov)){return;}
	if((oy < -128) || (!overflow)){ov = -ov;}
	oy+=ov;
}

static void widgetDrawTextLog(const widget *wid, textMesh *m, int x, int y, int w, int h){
	(void)w;
	(void)h;
	const int FS = 16;
	int i=0,bg=0;

	uint oldFont = m->font;
	m->font = 1;
	textMeshVGradient(m,x,y+h-2,w,2,0xA0301010,0xA0100000);
	for(int cy = y+h-2;cy>y;){
		const char *line = wid->valss[i++];
		if((line == NULL) || (*line == 0)){break;}

		uint lines = 1;
		for(const char *cl = line;*cl != 0;cl++){
			if(*cl == '\n'){lines++;}
		}
		cy -= lines * FS;
		if(bg){textMeshVGradient(m,x,cy,w,FS*lines,0x40301010,0x40100000);}
		if(*line == '>'){
			widgetDrawLispLine(m, x, cy, FS/8, w, FS, &line[2], 1, -1, -1);
			bg = !bg;
		}else{
			widgetDrawLispLine(m, x + FS*3, cy, FS/8, w, FS, &line[2], 0, -1, -1);
		}
	}
	m->font = oldFont;
}

void widgetDrawSingle(const widget *wid, textMesh *m,int x, int y, int w, int h){
	if(wid == NULL){return;}

	switch(wid->type){
	case wNone:
	case wSpace:
	case wGameScreen:
		break;
	case wPanel:
		widgetDrawPanel(wid,m,x,y,w,h);
		break;
	case wBackground:
		widgetDrawBackground(wid,m,x,y,w,h);
		break;
	case wHR:
		widgetDrawHR(wid,m,x,y,w,h);
		break;
	case wLabel:
		widgetDrawLabel(wid,m,x,y,w,h);
		break;
	case wTextInput:
		widgetDrawTextInput(wid,m,x,y,w,h);
		break;
	case wButton:
		widgetDrawButton(wid,m,x,y,w,h);
		break;
	case wButtonDel:
		widgetDrawButtondel(wid,m,x,y,w,h);
		break;
	case wRadioButton:
		widgetDrawRadioButton(wid,m,x,y,w,h);
		break;
	case wSlider:
		widgetDrawSlider(wid,m,x,y,w,h);
		break;
	case wItemSlot:
		widgetDrawItemSlot(wid,m,x,y,w,h);
		break;
	case wRecipeSlot:
		widgetDrawRecipeSlot(wid,m,x,y,w,h);
		break;
	case wRecipeInfo:
		widgetDrawRecipeInfo(wid,m,x,y,w,h);
		break;
	case wTextScroller:
		widgetDrawTextScroller(wid,m,x,y,w,h);
		break;
	case wTextLog:
		widgetDrawTextLog(wid,m,x,y,w,h);
		break;
	}
}
