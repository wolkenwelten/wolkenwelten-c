#pragma once
#include "../game/item.h"
#include "../../../common/src/network/packet.h"

void blockMiningMinePos(item *itm, int x, int y, int z);
void blockMiningInit();
void blockMiningDraw();
void blockMiningUpdateFromServer(packet *p);
