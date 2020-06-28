#pragma once
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/character.h"

void modsInit();

int   blockDamageDispatch   (const item *cItem, blockCategory blockCat);
bool  isSingleItemDispatch  (const item *cItem);
bool  hasMineActionDispatch (const item *cItem);
bool  mineActionDispatch    (      item *cItem, character *cChar, int to);
bool  activateItemDispatch  (      item *cItem, character *cChar);
