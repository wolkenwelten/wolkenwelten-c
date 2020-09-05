#pragma once
#include "../../../common/src/common.h"

void         itemDropNewP         (float x, float y, float z,const item *itm);
void         itemDropNewC         (const packet *p);
void         itemDropUpdate       ();
void         itemDropDel          (int d);
void         itemDropIntro        (int c);
uint8_t     *itemDropSaveChungus  (chungus *c, uint8_t *b);
void         itemDropDelChungus   (chungus *c);
uint8_t     *itemDropLoad         (uint8_t *b);
unsigned int itemDropUpdatePlayer (int c, unsigned int offset);
