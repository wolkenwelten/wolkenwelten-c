#pragma once
#include "../../../common/src/common.h"

typedef struct {
	item result;
	item ingredient[4];
} recipe;

extern recipe recipes[256];
extern uint recipeCount;

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

int characterGetItemOrSubstituteAmount(const character *c, u16 i);
int characterDecItemOrSubstituteAmount(character *c, u16 i, int a);
