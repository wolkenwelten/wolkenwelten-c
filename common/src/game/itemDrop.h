#pragma once
#include "../../../common/src/common.h"

extern itemDrop itemDropList[1<<14];
extern uint     itemDropCount;
extern int      itemDropFirstFree;

void itemDropNewC             (const character *chr,const item *itm);
void itemDropNewP             (const vec pos,const item *itm);
void itemDropUpdateAll        ();

itemDrop *itemDropGetByBeing  (being b);
being     itemDropGetBeing    (const itemDrop *id);
