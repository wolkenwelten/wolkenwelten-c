#pragma once
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/character.h"
#include "../gfx/mesh.h"

void  modsInit();
int   blockDamageDispatch   (item *cItem, blockCategory blockCat);
mesh *getMeshDispatch       (item *cItem);
float getInaccuracyDispatch (item *cItem);
bool  hasMineActionDispatch (item *cItem);
bool  mineActionDispatch    (item *cItem, character *cChar, int to);
bool  activateItemDispatch  (item *cItem, character *cChar, int to);
int   getAmmunitionDispatch (item *cItem);
int   getStackSizeDispatch  (item *cItem);

mesh *getMeshDefault      (item *cItem);
int   blockDamageDefault  (item *cItem, blockCategory blockCat);
bool  activateItemDefault (item *cItem, character *cChar, int to);
bool  mineActionDefault   (item *cItem, character *cChar, int to);
bool  hasMineActionDefault(item *cItem);
float getInaccuracyDefault(item *cItem);
int   getAmmunitionDefault(item *cItem);
int   getStackSizeDefault (item *cItem); 