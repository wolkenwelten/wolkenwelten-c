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

#include "lispLine.h"
#include "../../gfx/textMesh.h"

void widgetDrawTextLog(const widget *wid, textMesh *m, const box2D area){
	const int x = area.x;
	const int y = area.y;
	const int w = area.w;
	const int h = area.h;
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
			widgetDrawLispLine(m, rect(x, cy, w, FS), FS/8, &line[2], 1, -1, -1);
			bg = !bg;
		}else{
			widgetDrawLispLine(m, rect(x + FS*3, cy, w, FS), FS/8, &line[2], 0, -1, -1);
		}
	}
	m->font = oldFont;
}
