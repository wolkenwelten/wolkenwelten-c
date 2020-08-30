#pragma once
#include "../../../common/src/common.h"

void         itemDropNewP         (float x, float y, float z,const item *itm);
void         itemDropNewC         (const packet *p);
void         itemDropUpdate       ();
void         itemDropDel          (int d);
void         itemDropIntro        (int c);
void         itemDropDelChungus   (chungus *c);
unsigned int itemDropUpdatePlayer (int c, unsigned int offset);
