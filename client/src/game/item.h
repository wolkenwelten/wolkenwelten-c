#pragma once
#include "../game/blockType.h"
#include "../game/character.h"
#include "../gfx/mesh.h"

#include <stdbool.h>
#include <stdint.h>

item  itemNew        (uint16_t ID, int16_t amount);
item  itemEmpty      ();
void  itemDiscard    (item *i);
bool  itemIsEmpty    (item *i);
int   itemCanStack   (item *i, uint16_t ID);
int   itemIncStack   (item *i, int16_t amount);
int   itemDecStack   (item *i, int16_t amount);
bool  itemPlaceBlock (item *i, character *chr, int to);