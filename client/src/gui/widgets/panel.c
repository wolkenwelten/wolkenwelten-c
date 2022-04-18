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

#include "../../gfx/textMesh.h"

void widgetDrawPanel(const widget *wid, textMesh *m, box2D area){
	if(wid->flags & WIDGET_NO_BACKGROUND){return;}
	const int x = area.x;
	const int y = area.y;
	const int w = area.w;
	const int h = area.h;

	(void)wid;
	textMeshSolidBox (m,x+1,y+1,w-2,h-2,0xE0303030);

	u32 tcolor   = 0xFF777777;
	u32 bcolor   = 0xFF333333;
	textMeshSolidBox (m,x+1,y  ,w-2,  1,tcolor);
	textMeshSolidBox (m,x  ,y+1,  1,h-2,tcolor);
	textMeshSolidBox (m,x+1,y+h-1,w-2,  1,bcolor);
	textMeshSolidBox (m,x+w-1,y+1,  1,h-2,bcolor);
}
