#pragma once
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../network/packet.h"

void itemDropNewP         (float x, float y, float z,const item *itm);
void itemDropNewC         (const packetMedium *p);
void itemDropUpdate       ();
void itemDropUpdatePlayer (int c);
void itemDropDel          (int d);

