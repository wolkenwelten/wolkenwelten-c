#pragma once
#include "../game/blockType.h"
#include "../game/character.h"

#include <stdbool.h>
#include <stdint.h>

item  itemNew           (uint16_t ID, int16_t amount);
item  itemEmpty         ();
void  itemDiscard       (      item *i);
bool  itemIsEmpty       (const item *i);
int   itemBlockDamage   (const item *i, blockCategory cat);
bool  itemIsSingle      (const item *i);
bool  itemHasMineAction (const item *i);
bool  itemMineAction    (      item *i, character *chr, int to);
bool  itemActivate      (      item *i, character *chr);
bool  itemActivateBlock (      item *i, character *chr);
bool  itemCanStack      (const item *i, uint16_t ID);
bool  itemIncStack      (      item *i, int16_t amount);
bool  itemDecStack      (      item *i, int16_t amount);
