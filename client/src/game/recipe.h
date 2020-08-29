#pragma once
#include "../../../common/src/common.h"

void recipeAdd1I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID, unsigned char nIngredAmount);
void recipeAdd2I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2);
void recipeSetResult(int r,unsigned short nResultID,    unsigned char nResultAmount    );
void recipeAddIngred(int r,unsigned short nIngredientID,unsigned char nIngredientAmount);

unsigned short recipeGetResultID        (int r);
unsigned char  recipeGetResultAmount    (int r);
unsigned short recipeGetIngredientID    (int r, int i);
unsigned char  recipeGetIngredientAmount(int r, int i);

void           ingredientSubstituteAdd      (unsigned short ingredient, unsigned short substitute);
int            ingredientSubstituteGetAmount(unsigned short ingredient);
unsigned short ingredientSubstituteGetSub   (unsigned short ingredient, int i);

int  recipeGetCount();
int  recipeGetCraftableCount(character *c);
int  recipeGetCraftableIndex(character *c,int i);
int  recipeCanCraft(int r,character *c);
void recipeDoCraft (int r,character *c,int amount);

void recipeInit();
