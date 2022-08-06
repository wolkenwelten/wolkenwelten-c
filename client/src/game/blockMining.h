#pragma once
#include "../../../common/src/common.h"

void blockMiningInit             ();
void blockMiningDraw             ();
void blockMiningDropItemsPos     (int x, int y, int z, blockId b);
void blockMiningMineBlock        (int x, int y, int z, u8 cause);
int  blockMiningMinePos          (int dmg, int x, int y, int z);
void blockMiningUpdateAll        ();
void blockMiningUpdatePlayer     (uint c);
uint blockMiningGetActive        ();
