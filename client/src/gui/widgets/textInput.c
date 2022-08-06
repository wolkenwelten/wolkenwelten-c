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
#include "widgets.h"

#include "../textInput.h"
#include "../../gfx/textMesh.h"
#include "../../sdl/sdl.h"
#include "../../../../common/src/misc/misc.h"

void widgetDrawTextInput(const widget *wid, textMesh *m, const box2D area){
	const int x = area.x;
	const int y = area.y;
	const int w = area.w;
	const int h = area.h;

	u32 color    = 0xFF333333;
	u32 bcolor   = 0xFF555555;
	u32 tcolor   = 0xFF222222;
	int textYOff = (h - (2*8))/2;
	int textXOff = 8;
	int size     = 2;
	bool isLisp  = (wid->flags & WIDGET_LISP) != 0;

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

	if(wid->vals[0] == 0){
		u32 fgc = m->fgc;
		m->fgc &= 0x7FFFFFFF;
		textMeshAddStrPS(m,x+textXOff,y+textYOff,size,wid->label);
		m->fgc = fgc;
	}else{
		textMeshAddStrPS(m,x+textXOff,y+textYOff,size,wid->vals);
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
