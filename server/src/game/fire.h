#pragma once
#include "../../../common/src/game/fire.h"

void  fireIntro      (uint c);
void  fireRecvUpdate (uint c, const packet *p);
fire *fireGetAtPos   (u16 x,u16 y, u16 z);
void  fireUpdateAll  (uint off);
void  fireSyncPlayer (uint c);
