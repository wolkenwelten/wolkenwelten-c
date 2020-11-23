#pragma once
#include "../../../common/src/game/fire.h"

void fireDrawAll        ();
void fireRecvUpdate     (uint c, const packet *p);
void fireCheckPlayerBurn(uint off);
