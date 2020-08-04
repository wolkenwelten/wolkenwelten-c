#pragma once
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/character.h"
#include "../gfx/mesh.h"

void modsInit();

int   blockDamageDispatch   (item *cItem, blockCategory blockCat);
mesh *getMeshDispatch       (item *cItem);
bool  isSingleItemDispatch  (item *cItem);
bool  hasMineActionDispatch (item *cItem);
bool  mineActionDispatch    (item *cItem, character *cChar, int to);
bool  activateItemDispatch  (item *cItem, character *cChar, int to);
