#include "recipe.h"

#include <stdarg.h>

#include "../game/character.h"
#include "../game/item.h"

typedef struct {
	unsigned short resultID;
	unsigned char resultAmount;

	unsigned short ingredientID[4];
	unsigned char ingredientAmount[4];
} recipe;

recipe recipes[512];
int recipeCount = 0;

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
	recipes[r].resultID = nResultID;
	recipes[r].resultAmount = nResultAmount;
	recipes[r].ingredientID[0] = nIngredID;
	recipes[r].ingredientAmount[0] = nIngredAmount;
	recipes[r].ingredientID[1] = recipes[r].ingredientAmount[1] = 0;
}

void recipeAdd2I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2){
	int r = recipeCount++;
	recipes[r].resultID = nResultID;
	recipes[r].resultAmount = nResultAmount;
	recipes[r].ingredientID[0] = nIngredID1;
	recipes[r].ingredientAmount[0] = nIngredAmount1;
	recipes[r].ingredientID[1] = nIngredID2;
	recipes[r].ingredientAmount[1] = nIngredAmount2;
	recipes[r].ingredientID[2] = recipes[r].ingredientAmount[2] = 0;
}

int recipeCanCraft(int r,const character *c){
	if((r < 0) || (r >= recipeCount)){return 0;}
	int amount = 99999;
	for(int i=0;i<4;i++){
		if(recipes[r].ingredientID[i] == 0){continue;}
		if(recipes[r].ingredientAmount[i] == 0){continue;}

		const int camount = characterGetItemAmount(c,recipes[r].ingredientID[i]) / recipes[r].ingredientAmount[i];
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

		characterDecItemAmount(c,recipes[r].ingredientID[i],recipes[r].ingredientAmount[i]*amount);
	}
	characterPickupItem(c,recipes[r].resultID,amount*recipes[r].resultAmount);
}

void recipeInit(){
	recipeAdd1I(16,1,  5,1);
	recipeAdd1I(16,1, 10,1);
	recipeAdd1I(17,2, 16,1);
	recipeAdd1I(14,1, 12,1);
	recipeAdd1I(15,1, 12,1);
}
