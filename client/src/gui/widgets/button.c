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
#include "../../../../common/src/common.h"
#include "../widget.h"

#include "../../gfx/textMesh.h"

#include <string.h>

void widgetDrawButton(const widget *wid, textMesh *m, box2D area){
	u32 color    = 0xFF555555;
	u32 tcolor   = 0xFF777777;
	u32 bcolor   = 0xFF333333;
	int textYOff = (area.h - (2*8))/2;
	int textXOff = (area.w - (wid->label == NULL ? 0 : strnlen(wid->label,area.w/16)*16)) / 2;

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
	if(wid->flags & WIDGET_OUTLINED){
		tcolor = 0xFFBB7733;
		bcolor = 0xFF994411;
	}

	textMeshVGradient(m,area.x+1,area.y+1,area.w-2,area.h-2, color,bcolor);
	textMeshSolidBox (m,area.x+1,area.y  ,area.w-2,  1,tcolor);
	textMeshSolidBox (m,area.x  ,area.y+1,  1,area.h-2,tcolor);
	textMeshSolidBox (m,area.x+1,area.y+area.h-1,area.w-2,  1,bcolor);
	textMeshSolidBox (m,area.x+area.w-1,area.y+1,  1,area.h-2,bcolor);

	textMeshAddStrPS(m,area.x+textXOff,area.y+textYOff,2,wid->label);
}
