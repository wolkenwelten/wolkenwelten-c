#pragma once
#include "../../../common/src/common.h"

#define ITEM_DROPS_MAX (1<<12)
extern itemDrop itemDrops[ITEM_DROPS_MAX];
extern uint itemDropCount;

itemDrop *itemDropNew          ();
void      itemDropNewP         (const vec pos, const item *itm);
void      itemDropNewC         (uint c, const packet *p);
void      itemDropDel          (uint d);
void      itemDropDelChungus   (const chungus *c);
void      itemDropIntro        (uint c);
void      itemDropUpdate       ();
uint      itemDropUpdatePlayer (uint c, uint offset);
uint      itemDropGetActive    ();
