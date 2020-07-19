#pragma once
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../../../common/src/packet.h"

void itemDropNewP         (float x, float y, float z,const item *itm);
void itemDropNewC         (const packet *p);
void itemDropUpdate       ();
void itemDropUpdatePlayer (int c);
void itemDropDel          (int d);

