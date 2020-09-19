#pragma once
#include "../../../common/src/common.h"

void recipeAdd1I(u16 nResultID, u16 nResultAmount, u16 nIngredID,  u16 nIngredAmount);
void recipeAdd2I(u16 nResultID, u16 nResultAmount, u16 nIngredID1, u16 nIngredAmount1, u16 nIngredID2, u16 nIngredAmount2);
void recipeSetResult(uint r,u16 nResultID,     u16 nResultAmount);
void recipeAddIngred(uint r,u16 nIngredientID, u16 nIngredientAmount);

 u16 recipeGetResultID        (uint r);
 u16 recipeGetResultAmount    (uint r);
 u16 recipeGetIngredientID    (uint r, uint i);
 u16 recipeGetIngredientAmount(uint r, uint i);

void ingredientSubstituteAdd       (u16 ingredient, u16 substitute);
uint ingredientSubstituteGetAmount (u16 ingredient);
 u16 ingredientSubstituteGetSub    (u16 ingredient, uint i);

uint recipeGetCount          ();
uint recipeGetCraftableCount (const character *c);
 int recipeGetCraftableIndex (const character *c, uint i);
 int recipeCanCraft          (const character *c, uint r);
void recipeDoCraft           (      character *c, uint r, int amount);
void recipeInit              ();
