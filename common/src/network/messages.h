#pragma once
#include "../common.h"

#include <stdint.h>
#include "packet.h"
extern packet packetBuffer;

void msgRequestPlayerSpawnPos    ();
void msgPlayerSetPos             (int c, float x, float y, float z, float yaw, float pitch, float roll);
void msgRequestChungus           (int x, int y, int z);
void msgPlaceBlock               (int x, int y, int z, uint8_t b);
void msgMineBlock                (int x, int y, int z, uint8_t b);
void msgGoodbye                  ();
void msgBlockMiningUpdate        (int c, uint16_t x, uint16_t y, uint16_t z, uint16_t damage, int count, int i);
void msgSendChungusComplete      (int c, int x, int y, int z);
void msgCharacterGotHit          (int c,int pwr);
// 9 = playerSendName
void msgItemDropNew              (int c, float x, float y, float z, float vx, float vy, float vz, int ID, int amount);
void msgNewGrenade               (float x, float y, float z, float yaw, float pitch, float roll, float pwr);
void msgBeamBlast                (float x, float y, float z, float yaw, float pitch, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgPlayerMove               (int c, float dvx, float dvy, float dvz, float dyaw, float ypitch, float droll);
void msgCharacterHit             (int c, float x, float y, float z, float yaw, float pitch, float roll, int pwr);
// 15 = playerPos ???
// 16 = parseChatMsg ???
// 17 = parseDyingMsg ???
// 18 = chunkData ???
// 19 = setPlayerCount ???
void msgSetPlayerCount           (int playerLeaving, int playerMax);
void msgPickupItem               (int c, uint16_t ID, uint16_t amount);
void msgGrenadeExplode           (float x, float y, float z,float pwr, int style);
void msgGrenadeUpdate            (int c, float x, float y, float z, float vx, float vy, float vz, int count, int i);
void msgFxBeamBlaster            (int c, float x1, float y1, float z1, float x2, float y2, float z2, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgItemDropUpdate           (int c, float x, float y, float z, float vx, float vy, float vz, uint16_t i, uint16_t len, uint16_t itemID, uint16_t amount);
void msgPlayerDamage             (int c, int hp);
void msgUnsubChungus             (int x, int y, int z);
void msgPlayerSetData            (int c, int hp, int activeItem, uint32_t flags);
void msgPlayerSetInventory       (int c, item *itm, size_t itemCount);
