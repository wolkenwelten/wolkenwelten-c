#pragma once
#include "../../../common/src/common.h"

extern itemDrop itemDropList[1<<14];
extern uint     itemDropCount;
extern int      itemDropFirstFree;

void itemDropInit             ();
void itemDropNewC             (const character *chr,const item *itm);
void itemDropNewP             (const vec pos,const item *itm, i16 IDPlayer);
void itemDropDel              (uint d);
void itemDropDelChungus       (const chungus *c);
void itemDropUpdateAll        ();
void itemDropEmptyMsg         (uint c, uint i);
void itemDropNewP             (const vec pos,const item *itm, i16 IDPlayer);

itemDrop *itemDropGetByBeing  (being b);
being     itemDropGetBeing    (const itemDrop *id);
