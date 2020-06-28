#pragma once
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../network/packet.h"


void itemDropNewC(character *chr, item *itm);
void itemDropUpdate();
void itemDropUpdateFromServer(packetMedium *p);

