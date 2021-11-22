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

#include "../../game/character.h"
#include "../../game/recipe.h"
#include "../../gfx/textMesh.h"


void widgetDrawRecipeSlot(const widget *wid, textMesh *m, box2D area){
	const int x = area.x;
	const int y = area.y;
	const int w = area.w;
	const int h = area.h;

	int style = 0;
	const item recipeRes = recipeGetResult(wid->valu);
	i16 a = recipeCanCraft(player,wid->valu);
	if(a == 0){
		style = 2;
	}
	if((wid == widgetFocused) || (wid->flags & WIDGET_HOVER)){
		style = 1;
		widgetAddPopup(wid,area);
	}
	textMeshItemSlot(m,x,y,MIN(w,h),style,recipeRes.ID,a);
}
