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
#include "../../game/character/character.h"
#include "../../../../common/src/common.h"

void widgetDrawItemSlot(const widget *wid, textMesh *m, box2D area, const item *itm){
	const int x = area.x;
	const int y = area.y;
	const int w = area.w;
	int style = 0;
	if((wid == widgetFocused) || (wid->flags & WIDGET_HOVER)){
		widgetAddPopup(wid,area);
	}
	if((wid == widgetFocused) || (wid->flags & WIDGET_HOVER) || (itm == characterGetActiveItem(player))){
		style = 1;
	}
	textMeshItem(m, x, y, w, style, itm);
}
