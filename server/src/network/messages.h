#pragma once
#include "../game/item.h"
#include "../network/packet.h"

#include <stdint.h>

void msgPickupItem            (int c, const item *i);
void msgMineBlock             (int x, int y, int z, uint8_t b);
void msgGrenadeExplode        (float x, float y, float z,float pwr, int style);
void msgFxBeamBlaster         (int c, float x1, float y1, float z1, float x2, float y2, float z2, float pwr);
void msgPlayerMove            (int c, float dvx, float dvy, float dvz, float dyaw, float ypitch, float droll);
void msgUpdatePlayer          (int c);
void msgRequestPlayerSpawnPos (int c, const packetSmall *p);
void msgRequestChungus        (int c, const packetSmall *p);
void msgSendChungusComplete   (int c, int x, int y, int z);
void msgSendChunk             (int c, const chunk *chnk);
