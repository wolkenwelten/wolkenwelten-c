#pragma once
#include "../../../common/src/common.h"

extern int itemDropCount;

void itemDropNewC             (const character *chr,const item *itm);
void itemDropNewP             (const vec pos,const item *itm);
void itemDropUpdate           ();
void itemDropDel              (int d);
void itemDropNewFromServer    (const packet *p);
void itemDropUpdateFromServer (const packet *p);
