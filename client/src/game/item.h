#pragma once
#include "../game/blockType.h"
#include "../game/character.h"
#include "../gfx/mesh.h"

#include <stdbool.h>
#include <stdint.h>

item  itemNew              (uint16_t ID, int16_t amount);
item  itemEmpty            ();
void  itemDiscard          (item *i);
bool  itemIsEmpty          (item *i);
int   itemBlockDamage      (item *i, blockCategory cat);
float itemGetInaccuracy    (item *i);
mesh *itemGetMesh          (item *i);
bool  itemIsSingle         (item *i);
bool  itemHasPrimaryAction (item *i);
bool  itemPrimaryAction    (item *i, character *chr, int to);
bool  itemSecondaryAction  (item *i, character *chr, int to);
bool  itemTertiaryAction   (item *i, character *chr, int to);
bool  itemPlaceBlock       (item *i, character *chr, int to);
int   itemCanStack         (item *i, uint16_t ID);
int   itemIncStack         (item *i, int16_t amount);
int   itemDecStack         (item *i, int16_t amount);
