#include "recipe.h"
#include "../game/character.h"

typedef struct {
	u16 resultID;
	u16 resultAmount;

	u16 ingredientID[4];
	u16 ingredientAmount[4];
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

u16 recipeGetResultID(uint r){
	if(r >= recipeCount){return 0;}
	return recipes[r].resultID;
}
u16 recipeGetResultAmount(uint r){
	if(r >= recipeCount){return 0;}
	return recipes[r].resultAmount;
}
u16 recipeGetIngredientID(uint r,uint i){
	if(r >= recipeCount) { return 0; }
	if(i >= 4)           { return 0; }
	return recipes[r].ingredientID[i];
}
u16 recipeGetIngredientAmount(uint r,uint i){
	if(r >= recipeCount) { return 0; }
	if(i >= 4)           { return 0; }
	return recipes[r].ingredientAmount[i];
}

void recipeSetResult(uint r,u16 nResultID,u16 nResultAmount){
	if(r >= recipeCount) { return; }
	recipes[r].resultID = nResultID;
	recipes[r].resultAmount = nResultAmount;
	recipeCount++;
}

void recipeAddIngred(uint r,u16 nIngredientID,u16 nIngredientAmount){
	if(r >= recipeCount) { return; }
	for(int i=0;i<4;i++){
		if((recipes[r].ingredientID[i] == 0) || (recipes[r].ingredientAmount[i] == 0)){
			recipes[r].ingredientID[i]     = nIngredientID;
			recipes[r].ingredientAmount[i] = nIngredientAmount;
			return;
		}
	}
}

void recipeAdd1I(u16 nResultID, u16 nResultAmount, u16 nIngredID, u16 nIngredAmount){
	int r = recipeCount++;
	recipes[r].resultID            = nResultID;
	recipes[r].resultAmount        = nResultAmount;
	recipes[r].ingredientID[0]     = nIngredID;
	recipes[r].ingredientAmount[0] = nIngredAmount;
	recipes[r].ingredientID[1]     = recipes[r].ingredientAmount[1] = 0;
}

void recipeAdd2I(u16 nResultID, u16 nResultAmount, u16 nIngredID1, u16 nIngredAmount1, u16 nIngredID2, u16 nIngredAmount2){
	int r = recipeCount++;
	recipes[r].resultID            = nResultID;
	recipes[r].resultAmount        = nResultAmount;
	recipes[r].ingredientID[0]     = nIngredID1;
	recipes[r].ingredientAmount[0] = nIngredAmount1;
	recipes[r].ingredientID[1]     = nIngredID2;
	recipes[r].ingredientAmount[1] = nIngredAmount2;
	recipes[r].ingredientID[2]     = recipes[r].ingredientAmount[2] = 0;
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
		if(recipes[r].ingredientID[i]     == 0){continue;}
		if(recipes[r].ingredientAmount[i] == 0){continue;}

		const int camount = characterGetItemOrSubstituteAmount(c,recipes[r].ingredientID[i]) / recipes[r].ingredientAmount[i];
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
		if(recipes[r].ingredientID[i]     == 0){continue;}
		if(recipes[r].ingredientAmount[i] == 0){continue;}

		characterDecItemOrSubstituteAmount(c,recipes[r].ingredientID[i],recipes[r].ingredientAmount[i]*amount);
	}
	characterPickupItem(c,recipes[r].resultID,amount*recipes[r].resultAmount);
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
	recipeAdd1I(17,2, 10,1);
	recipeAdd1I(14,1, 12,1);
	recipeAdd1I(15,1, 12,1);
}
