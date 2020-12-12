#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/itemDrop.h"

itemDrop *itemDropNew          ();
void      itemDropDel          (uint d);
void      itemDropNewPacket    (uint c, const packet *p);
void      itemDropPickupP      (uint c, const packet *p);
void      itemDropDelChungus   (const chungus *c);
void      itemDropIntroChungus (uint c, const chungus *chng);
void      itemDropUpdateFire   (uint i);
void      itemDropUpdateFireAll();
uint      itemDropUpdatePlayer (uint c, uint offset);
uint      itemDropGetActive    ();
