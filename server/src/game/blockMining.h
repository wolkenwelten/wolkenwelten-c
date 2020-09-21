#pragma once
#include "../../../common/src/common.h"

void blockMiningDropItemsPos (int x, int y, int z, u8 b);
int  blockMiningMinePos      (item *itm, int x, int y, int z);
void blockMiningUpdate       ();
void blockMiningUpdatePlayer (uint c);
uint blockMiningGetActive    ();
