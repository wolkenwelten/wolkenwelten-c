#pragma once
#include "../game/item.h"

void blockMiningDropItemsPos (int x, int y, int z, uint8_t b);
void blockMiningMinePos      (item *itm, int x, int y, int z);
void blockMiningUpdate       ();
void blockMiningUpdatePlayer (int c);
