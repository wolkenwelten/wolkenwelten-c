#pragma once
#include "../game/item.h"
#include "../network/packet.h"

void blockMiningMinePos(item *itm, int x, int y, int z);
void blockMiningInit();
void blockMiningDraw();
void blockMiningUpdateFromServer(packetSmall *p);
