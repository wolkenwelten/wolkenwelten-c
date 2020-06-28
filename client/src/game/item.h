#pragma once
#include "../game/blockType.h"
#include "../game/character.h"
#include "../gfx/mesh.h"

#include <stdbool.h>
#include <stdint.h>

item  itemNew           (uint16_t ID, int16_t amount);
item  itemEmpty         ();
void  itemDiscard       (item *i);
bool  itemIsEmpty       (item *i);
int   itemBlockDamage   (item *i, blockCategory cat);
mesh *itemGetMesh       (item *i);
bool  itemIsSingle      (item *i);
bool  itemHasMineAction (item *i);
bool  itemMineAction    (item *i, character *chr, int to);
bool  itemActivate      (item *i, character *chr);
bool  itemActivateBlock (item *i, character *chr);
bool  itemCanStack      (item *i, uint16_t ID);
bool  itemIncStack      (item *i, int16_t amount);
bool  itemDecStack      (item *i, int16_t amount);
