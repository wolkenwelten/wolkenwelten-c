#pragma once
#include "../../../common/src/common.h"

void blockMiningBurnBlock    (int x, int y, int z, blockId b);
void blockMiningDropItemsPos (int x, int y, int z, blockId b);
void blockMiningMineBlock    (int x, int y, int z, u8 cause);
int  blockMiningMinePos      (int dmg, int x, int y, int z);
int  blockMiningMinePosItem  (item *itm, int x, int y, int z);
void blockMiningUpdateAll    ();
void blockMiningUpdatePlayer (uint c);
uint blockMiningGetActive    ();
