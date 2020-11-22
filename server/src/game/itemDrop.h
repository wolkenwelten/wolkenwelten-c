#pragma once
#include "../../../common/src/common.h"

extern itemDrop itemDropList[1<<12];
extern uint itemDropCount;

itemDrop *itemDropNew          ();
void      itemDropNewP         (const vec pos, const item *itm);
void      itemDropNewC         (uint c, const packet *p);
void      itemDropPickupP      (uint c, const packet *p);
void      itemDropDel          (uint d);
void      itemDropDelChungus   (const chungus *c);
void      itemDropIntro        (uint c);
void      itemDropUpdate       ();
void      itemDropUpdateFire   (uint off);
uint      itemDropUpdatePlayer (uint c, uint offset);
uint      itemDropGetActive    ();
