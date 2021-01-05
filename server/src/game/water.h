#pragma once
#include "../../../common/src/game/water.h"

void  waterNewF         (u16 x, u16 y, u16 z, i16 amount);
void  waterRecvUpdate   (uint c, const packet *p);
void  waterUpdateAll    ();
void  waterSyncPlayer   (uint c);
void waterDelChungus    (const chungus *c);
