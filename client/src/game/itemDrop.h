#pragma once
#include "../../../common/src/common.h"

extern int itemDropCount;

void itemDropNewC(character *chr, item *itm);
void itemDropNewD(float x, float y, float z, item *itm);
void itemDropUpdate();
void itemDropDel(int d);
void itemDropNewFromServer(packet *p);
void itemDropUpdateFromServer(packet *p);
