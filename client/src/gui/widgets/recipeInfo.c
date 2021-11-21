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
#include "../../sdl/sdl.h"
#include "../../game/recipe.h"
#include "../../../../common/src/game/item.h"

#include <math.h>
#include <string.h>

void widgetDrawRecipeInfo(const widget *wid, textMesh *m, int x, int y, int w, int h){
	const uint ticks = getTicks()>>4;
	const uint ts    = MIN(w,h);
	uint ii,xx,r = wid->vali;
	if(r >= recipeGetCount()){return;}
	const int animX = sinf((float)ticks/24.f)*ts/8;
	const int animY = cosf((float)ticks/24.f)*ts/8;
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
		const char *name = itemGetName(&ingred);
		const uint len = strnlen(name,256);
		const int yoff = ii & 1 ? 0 : ts/3;
		int xoff = MAX((int)(x-ts/4),(int)(xx+ts/2-(len*8)));
		textMeshAddStrPS(m,xoff,y+ts+yoff+ts/6,2,name);
	}

	xx = ii*2*ts + x;
	textMeshBox(m,xx-ts+ts/4+animX*2,y+ts/4,ts/2,ts/2,25.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
	textMeshItemSlot(m,xx,y,ts,3,result.ID,result.amount);
	const char *name = itemGetName(&result);
	if(name == NULL){return;}
	const uint len = strnlen(name,256);
	const int yoff = ii & 1 ? 0 : ts/3;
	textMeshAddStrPS(m,xx+ts/2-(len*8),y+ts+yoff+ts/6,2,name);
}
