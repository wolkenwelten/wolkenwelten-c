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

#include <math.h>
#include <stdlib.h>

void widgetDrawTextScroller(const widget *wid, textMesh *m, const box2D area){
	const int x = area.x;
	const int y = area.y;

	static int oy = -128;
	static int ov = 2;

	m->wrap = 1;
	bool overflow = textMeshPrintfPS(m,x+16,y+16-oy,2,"%s",wid->label);
	m->wrap = 0;

	if(!overflow && (abs(oy) < ov)){return;}
	if((oy < -128) || (!overflow)){ov = -ov;}
	oy+=ov;
}
