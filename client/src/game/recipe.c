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

#include "recipe.h"
#include "../game/character.h"
#include "../../../common/src/game/item.h"

struct ingredientSubstitute;
struct ingredientSubstitute {
	u16 ingredient;
	u16 substitute;

	struct ingredientSubstitute *next;
};
typedef struct ingredientSubstitute ingredientSubstitute;

recipe recipes[256];
uint recipeCount = 0;

ingredientSubstitute *substitutes[512];
ingredientSubstitute substitutePool[64];
uint substitutePoolUsed=0;

void ingredientSubstituteAdd(u16 ingredient, u16 substitute){
	ingredientSubstitute *s,*sub = &substitutePool[substitutePoolUsed++];

	sub->ingredient = ingredient;
	sub->substitute = substitute;
	sub->next       = NULL;

	if(substitutes[ingredient] == NULL){
		substitutes[ingredient] = sub;
	}else{
		for(s = substitutes[ingredient];s->next != NULL;s = s->next){}
		s->next = sub;
	}
}

uint ingredientSubstituteGetAmount(u16 ingredient){
	uint ret = 0;

	for(ingredientSubstitute *s = substitutes[ingredient];s != NULL;s = s->next){
		ret++;
	}

	return ret;
}

u16 ingredientSubstituteGetSub(u16 ingredient, uint i){
	for(ingredientSubstitute *s = substitutes[ingredient];s != NULL;s = s->next){
		if(i-- == 0){ return s->substitute; }
	}
	return ingredient;
}

uint recipeGetCount(){
	return recipeCount;
}

item recipeGetResult(uint r){
	if(r >= recipeCount){return itemEmpty();}
	return recipes[r].result;
}

item recipeGetIngredient(uint r,uint i){
	if(r >= recipeCount) { return itemEmpty(); }
	if(i >= 4)           { return itemEmpty(); }
	return recipes[r].ingredient[i];
}

int characterGetItemOrSubstituteAmount(const character *c, u16 i){
	ingredientSubstitute *s;
	int ret = characterGetItemAmount(c,i);
	if(i >= countof(substitutes)){return 0;}

	for(s = substitutes[i];s != NULL;s = s->next){
		ret += characterGetItemAmount(c,s->substitute);
	}
	return ret;
}

int characterDecItemOrSubstituteAmount(character *c, u16 i, int a){
	ingredientSubstitute *s;
	int retAmount = a;
	int ret = characterGetItemAmount(c,i);
	a -= characterDecItemAmount(c,i,MIN(ret,a));
	if(a <= 0){return retAmount;}

	for(s = substitutes[i];s != NULL;s = s->next){
		ret = characterGetItemAmount(c,s->substitute);
		a  -= characterDecItemAmount(c,s->substitute,MIN(ret,a));
		if(a <= 0){return retAmount;}
	}
	return retAmount-a;
}

int recipeCanCraft(const character *c, uint r){
	if(r >= recipeCount){return 0;}
	int amount = 99999;
	for(int i=0;i<4;i++){
		if(itemIsEmpty(&recipes[r].ingredient[i])){continue;}
		const int camount = characterGetItemOrSubstituteAmount(c,recipes[r].ingredient[i].ID) / recipes[r].ingredient[i].amount;
		if(camount < amount){amount = camount;}
	}
	if(amount >= 9999){return 0;}
	return amount;
}

void recipeDoCraft(character *c, uint r, int amount){
	if(r >= recipeCount){return;}
	int canCraftAmount = recipeCanCraft(c,r);
	if(canCraftAmount == 0){return;}
	if(canCraftAmount < amount){amount = canCraftAmount;}

	for(int i=0;i<4;i++){
		if(itemIsEmpty(&recipes[r].ingredient[i])){continue;}

		characterDecItemOrSubstituteAmount(c,recipes[r].ingredient[i].ID,recipes[r].ingredient[i].amount*amount);
	}
	characterPickupItem(c,recipes[r].result.ID,amount*recipes[r].result.amount);
}

uint recipeGetCraftableCount(const character *c){
	uint ret=0;
	for(uint r=0;r<recipeCount;r++){
		if(recipeCanCraft(c,r) > 0){ret++;}
	}
	return ret;
}

 int recipeGetCraftableIndex(const character *c,uint i){
	uint ret=0;
	for(uint r=0;r<recipeCount;r++){
		if(recipeCanCraft(c,r) > 0){
			if(ret++ == i){ return r; }
		}
	}
	return -1;
}

void recipeInit(){
	ingredientSubstituteAdd(I_Oak,I_Spruce);
	ingredientSubstituteAdd(I_Oak,I_Birch);
}
