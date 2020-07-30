#pragma once
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../../../common/src/network/packet.h"

void         itemDropNewP         (float x, float y, float z,const item *itm);
void         itemDropNewC         (const packet *p);
void         itemDropUpdate       ();
void         itemDropDel          (int d);
void         itemDropIntro        (int c);
unsigned int itemDropUpdatePlayer (int c, unsigned int offset);
