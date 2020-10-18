#pragma once
#include "../common.h"

#include "packet.h"
extern packet packetBuffer;

void msgNOP                      (uint len);
void msgRequestPlayerSpawnPos    ();
void msgPlayerSetPos             (uint c, const vec pos, const vec rot);
void msgRequestChungus           (u8 x, u8 y, u8 z);
void msgUnsubChungus             (u8 x, u8 y, u8 z);
void msgDirtyChunk               (u16 x, u16 y, u16 z);
void msgPlaceBlock               (u16 x, u16 y, u16 z, u8 b);
void msgMineBlock                (u16 x, u16 y, u16 z, u8 b);
void msgGoodbye                  ();
void msgBlockMiningUpdate        (uint c, u16 x, u16 y, u16 z, i16 damage, u16 count, u16 i);
void msgSendChungusComplete      (uint c, u8 x, u8 y, u8 z);
void msgBeingGotHit              (        i16 hp, u16 cause, being target, being culprit);
void msgBeingDamage              (uint c, i16 hp, u16 cause, being target, being culprit, const vec pos);
// 9 = playerSendName
void msgItemDropNew              (uint c, const vec pos, const vec vel, const item *itm);
void msgNewGrenade               (const vec pos, const vec rot, float pwr, int cluster, float clusterPwr);
void msgBeamBlast                (const vec pos, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgPlayerMove               (uint c, const vec dpos, const vec drot);
void msgPlayerName               (uint c, u16 i, const char *name);
// 15 = playerPos ???
// 16 = parseChatMsg ???
// 17 = parseDyingMsg ???
// 18 = chunkData ???
// 19 = setPlayerCount ???
void msgSetPlayerCount           (u16 playerLeaving, u16 playerMax);
void msgPickupItem               (uint c, const item itm);
void msgGrenadeExplode           (const vec pos,float pwr, u16 style);
void msgGrenadeUpdate            (uint c, const vec pos, const vec vel, u16 i, u16 count);
void msgFxBeamBlaster            (uint c, const vec pa, const vec pb, float beamSize, float damageMultiplier);
void msgItemDropUpdate           (uint c, const vec pos, const vec vel, const item *itm, u16 i, u16 len);
void msgPlayerSetData            (uint c, i16 hp, u16 activeItem, u32 flags);
void msgPlayerSetInventory       (uint c, const item *itm, size_t itemCount);
void msgPingPong                 (uint c);
