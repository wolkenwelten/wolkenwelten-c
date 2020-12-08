#pragma once
#include "../../../common/src/game/water.h"

void waterDrawAll        ();
void waterRecvUpdate     (uint c, const packet *p);
void waterCheckPlayerBurn(uint off);
