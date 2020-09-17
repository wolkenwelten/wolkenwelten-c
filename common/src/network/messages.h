#pragma once
#include "../common.h"

#include <stdint.h>
#include "packet.h"
extern packet packetBuffer;

void msgRequestPlayerSpawnPos    ();
void msgPlayerSetPos             (int c, const vec pos, const vec rot);
void msgRequestChungus           (int x, int y, int z);
void msgPlaceBlock               (int x, int y, int z, uint8_t b);
void msgMineBlock                (int x, int y, int z, uint8_t b);
void msgGoodbye                  ();
void msgBlockMiningUpdate        (int c, uint16_t x, uint16_t y, uint16_t z, uint16_t damage, int count, int i);
void msgSendChungusComplete      (int c, int x, int y, int z);
void msgCharacterGotHit          (int c, int pwr);
// 9 = playerSendName
void msgItemDropNew              (int c, const vec pos, const vec vel, const item *itm);
void msgNewGrenade               (const vec pos, const vec rot, float pwr);
void msgBeamBlast                (const vec pos, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgPlayerMove               (int c, const vec dpos, const vec drot);
void msgCharacterHit             (int c, const vec pos, const vec rot, int pwr);
// 15 = playerPos ???
// 16 = parseChatMsg ???
// 17 = parseDyingMsg ???
// 18 = chunkData ???
// 19 = setPlayerCount ???
void msgSetPlayerCount           (int playerLeaving, int playerMax);
void msgPickupItem               (int c, uint16_t ID, uint16_t amount);
void msgGrenadeExplode           (const vec pos,float pwr, int style);
void msgGrenadeUpdate            (int c, const vec pos, const vec vel, int count, int i);
void msgFxBeamBlaster            (int c, const vec pa, const vec pb, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgItemDropUpdate           (int c, const vec pos, const vec vel, uint16_t i, uint16_t len, uint16_t itemID, uint16_t amount);
void msgPlayerDamage             (int c, int hp);
void msgUnsubChungus             (int x, int y, int z);
void msgPlayerSetData            (int c, int hp, int activeItem, uint32_t flags);
void msgPlayerSetInventory       (int c, const item *itm, size_t itemCount);
