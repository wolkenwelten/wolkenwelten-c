#pragma once
#include "../../../common/src/common.h"

void blockMiningMinePos          (const item *itm, int x, int y, int z);
void blockMiningInit             ();
void blockMiningDraw             ();
void blockMiningUpdateFromServer (const packet *p);
