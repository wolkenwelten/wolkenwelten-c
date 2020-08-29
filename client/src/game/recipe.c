#include "recipe.h"
#include "../game/character.h"

typedef struct {
	unsigned short resultID;
	unsigned char resultAmount;

	unsigned short ingredientID[4];
	unsigned char ingredientAmount[4];
} recipe;

struct ingredientSubstitute;
struct ingredientSubstitute {
	unsigned short ingredient;
	unsigned short substitute;

	struct ingredientSubstitute *next;
};
typedef struct ingredientSubstitute ingredientSubstitute;

recipe recipes[16];
int recipeCount = 0;

ingredientSubstitute *substitutes[512];
ingredientSubstitute substitutePool[16];
int substitutePoolUsed=0;

void ingredientSubstituteAdd(unsigned short ingredient, unsigned short substitute){
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

int ingredientSubstituteGetAmount(unsigned short ingredient){
	int ret = 0;

	for(ingredientSubstitute *s = substitutes[ingredient];s != NULL;s = s->next){
		ret++;
	}

	return ret;
}

unsigned short ingredientSubstituteGetSub(unsigned short ingredient, int i){
	for(ingredientSubstitute *s = substitutes[ingredient];s != NULL;s = s->next){
		if(i-- == 0){ return s->substitute; }
	}
	return ingredient;
}

int recipeGetCount(){
	return recipeCount;
}

unsigned short recipeGetResultID(int r){
	if((r < 0) || (r >= recipeCount)){return 0;}
	return recipes[r].resultID;
}
unsigned char  recipeGetResultAmount(int r){
	if((r < 0) || (r >= recipeCount)){return 0;}
	return recipes[r].resultAmount;
}
unsigned short recipeGetIngredientID(int r,int i){
	if((r < 0) || (r >= recipeCount)){return 0;}
	if((i < 0) || (i >= 4)){return 0;}
	return recipes[r].ingredientID[i&3];
}
unsigned char  recipeGetIngredientAmount(int r,int i){
	if((r < 0) || (r >= recipeCount)){return 0;}
	if((i < 0) || (i >= 4)){return 0;}
	return recipes[r].ingredientAmount[i&3];
}

void recipeSetResult(int r,unsigned short nResultID,unsigned char nResultAmount){
	recipes[r].resultID = nResultID;
	recipes[r].resultAmount = nResultAmount;
	recipeCount++;
}

void recipeAddIngred(int r,unsigned short nIngredientID,unsigned char nIngredientAmount){
	for(int i=0;i<4;i++){
		if((recipes[r].ingredientID[i] == 0) || (recipes[r].ingredientAmount[i] == 0)){
			recipes[r].ingredientID[i] = nIngredientID;
			recipes[r].ingredientAmount[i] = nIngredientAmount;
			return;
		}
	}
}

void recipeAdd1I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID, unsigned char nIngredAmount){
	int r = recipeCount++;
	recipes[r].resultID            = nResultID;
	recipes[r].resultAmount        = nResultAmount;
	recipes[r].ingredientID[0]     = nIngredID;
	recipes[r].ingredientAmount[0] = nIngredAmount;
	recipes[r].ingredientID[1]     = recipes[r].ingredientAmount[1] = 0;
}

void recipeAdd2I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2){
	int r = recipeCount++;
	recipes[r].resultID            = nResultID;
	recipes[r].resultAmount        = nResultAmount;
	recipes[r].ingredientID[0]     = nIngredID1;
	recipes[r].ingredientAmount[0] = nIngredAmount1;
	recipes[r].ingredientID[1]     = nIngredID2;
	recipes[r].ingredientAmount[1] = nIngredAmount2;
	recipes[r].ingredientID[2]     = recipes[r].ingredientAmount[2] = 0;
}

#include <stdio.h>
int characterGetItemOrSubstituteAmount(character *c, unsigned short i){
	ingredientSubstitute *s;
	int ret = characterGetItemAmount(c,i);
	if(i >= (sizeof(substitutes)/sizeof(ingredientSubstitute *))){return 0;}

	for(s = substitutes[i];s != NULL;s = s->next){
		ret += characterGetItemAmount(c,s->substitute);
	}
	return ret;
}

int characterDecItemOrSubstituteAmount(character *c, unsigned short i, int a){
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

int recipeCanCraft(int r,character *c){
	if((r < 0) || (r >= recipeCount)){return 0;}
	int amount = 99999;
	for(int i=0;i<4;i++){
		if(recipes[r].ingredientID[i] == 0){continue;}
		if(recipes[r].ingredientAmount[i] == 0){continue;}

		const int camount = characterGetItemOrSubstituteAmount(c,recipes[r].ingredientID[i]) / recipes[r].ingredientAmount[i];
		if(camount < amount){amount = camount;}
	}
	if(amount  < 0){return 0;}
	if(amount >= 99999){return 0;}
	return amount;
}

void recipeDoCraft(int r, character *c,int amount){
	if((r < 0) || (r >= recipeCount)){return;}
	if(r >= recipeCount){return;}
	int canCraftAmount = recipeCanCraft(r,c);
	if(canCraftAmount == 0){return;}
	if(canCraftAmount < amount){amount = canCraftAmount;}

	for(int i=0;i<4;i++){
		if(recipes[r].ingredientID[i] == 0){continue;}
		if(recipes[r].ingredientAmount[i] == 0){continue;}

		characterDecItemOrSubstituteAmount(c,recipes[r].ingredientID[i],recipes[r].ingredientAmount[i]*amount);
	}
	characterPickupItem(c,recipes[r].resultID,amount*recipes[r].resultAmount);
}

int  recipeGetCraftableCount(character *c){
	int ret=0;
	for(int r=0;r<recipeCount;r++){
		if(recipeCanCraft(r,c) > 0){ret++;}
	}
	return ret;
}
int recipeGetCraftableIndex(character *c,int i){
	int ret=0;
	for(int r=0;r<recipeCount;r++){
		if(recipeCanCraft(r,c) > 0){
			if(ret++ == i){
				return r;	
			}
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
