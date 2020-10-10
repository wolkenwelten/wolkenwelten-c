#include "recipe.h"
#include "../game/character.h"
#include "../../../common/src/game/item.h"

typedef struct {
	item result;
	item ingredient[4];
} recipe;

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

void recipeNew1(const item result, const item ingred1){
	int r = recipeCount++;
	recipes[r].result = result;
	recipes[r].ingredient[0] = ingred1;
	recipes[r].ingredient[1] = itemEmpty();
}

void recipeNew2(const item result, const item ingred1, const item ingred2){
	int r = recipeCount++;
	recipes[r].result = result;
	recipes[r].ingredient[0] = ingred1;
	recipes[r].ingredient[1] = ingred2;
	recipes[r].ingredient[2] = itemEmpty();
}

void recipeNew3(const item result, const item ingred1, const item ingred2, const item ingred3){
	int r = recipeCount++;
	recipes[r].result = result;
	recipes[r].ingredient[0] = ingred1;
	recipes[r].ingredient[1] = ingred2;
	recipes[r].ingredient[2] = ingred3;
	recipes[r].ingredient[3] = itemEmpty();
}

void recipeNew4(const item result, const item ingred1, const item ingred2, const item ingred3, const item ingred4){
	int r = recipeCount++;
	recipes[r].result = result;
	recipes[r].ingredient[0] = ingred1;
	recipes[r].ingredient[1] = ingred2;
	recipes[r].ingredient[2] = ingred3;
	recipes[r].ingredient[3] = ingred4;
}


int characterGetItemOrSubstituteAmount(const character *c, u16 i){
	ingredientSubstitute *s;
	int ret = characterGetItemAmount(c,i);
	if(i >= (sizeof(substitutes)/sizeof(ingredientSubstitute *))){return 0;}

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
	ingredientSubstituteAdd(10,5);
	ingredientSubstituteAdd(10,20);
	//recipeAdd1I(16,1, 10,1);
	recipeNew1(itemNew(17,2), itemNew(10,1));
	recipeNew1(itemNew(14,1), itemNew(12,1));
	recipeNew1(itemNew(15,1), itemNew(12,1));
}
