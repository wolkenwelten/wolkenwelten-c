#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/itemDrop.h"

itemDrop *itemDropNew          ();
void      itemDropNewPacket    (uint c, const packet *p);
void      itemDropPickupP      (uint c, const packet *p);
void      itemDropBounceP      (uint c, const packet *p);
void      itemDropUpdateFire   (uint i);
uint      itemDropUpdatePlayer (uint c, uint offset);
uint      itemDropGetActive    ();
void      itemDropUpdateMsg    (u8 c,uint i);
