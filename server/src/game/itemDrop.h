#pragma once
#include "../../../common/src/common.h"

extern int   itemDropCount;

void         itemDropNewP         (const vec pos, const item *itm);
void         itemDropNewC         (const packet *p);
void         itemDropUpdate       ();
void         itemDropDel          (int d);
void         itemDropIntro        (int c);
void        *itemDropSaveChungus  (chungus *c, void *buf);
void         itemDropDelChungus   (chungus *c);
void        *itemDropLoad         (void *buf);
unsigned int itemDropUpdatePlayer (int c, unsigned int offset);
