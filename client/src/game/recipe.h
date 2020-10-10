#pragma once
#include "../../../common/src/common.h"

void recipeNew1 (const item result, const item ingred1);
void recipeNew2 (const item result, const item ingred1, const item ingred2);
void recipeNew3 (const item result, const item ingred1, const item ingred2, const item ingred3);
void recipeNew4 (const item result, const item ingred1, const item ingred2, const item ingred3, const item ingred4);

item recipeGetResult     (uint r);
item recipeGetIngredient (uint r, uint i);

void ingredientSubstituteAdd       (u16 ingredient, u16 substitute);
uint ingredientSubstituteGetAmount (u16 ingredient);
 u16 ingredientSubstituteGetSub    (u16 ingredient, uint i);

uint recipeGetCount          ();
uint recipeGetCraftableCount (const character *c);
 int recipeGetCraftableIndex (const character *c, uint i);
 int recipeCanCraft          (const character *c, uint r);
void recipeDoCraft           (      character *c, uint r, int amount);
void recipeInit              ();
