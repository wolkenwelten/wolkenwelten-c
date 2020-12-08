#pragma once
#include "../../../common/src/game/water.h"

void  fireNewF         (u16 x, u16 y, u16 z, i16 strength, i16 blockDmg);
void  fireIntroChungus (uint c, const chungus *chng);
void  fireRecvUpdate   (uint c, const packet *p);
void  fireUpdateAll    ();
void  fireSyncPlayer   (uint c);
