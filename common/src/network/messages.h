#pragma once
#include "../common.h"

#include "packet.h"
extern packet packetBuffer;

void msgNOP                      (uint len);
void msgRequestPlayerSpawnPos    ();
void msgPlayerSetPos             (int c, const vec pos, const vec rot, const vec vel);
void msgRequestChungus           (int c, u8  x,  u8 y,  u8 z);
void msgUnsubChungus             (int c, u8  x,  u8 y,  u8 z);
void msgDirtyChunk               (u16 x, u16 y, u16 z);
void msgPlaceBlock               (u16 x, u16 y, u16 z, u8 b);
void msgMineBlock                (u16 x, u16 y, u16 z, u8 b, u8 cause);
void msgGoodbye                  (int c);
void msgBlockMiningUpdate        (int c, u16 x, u16 y, u16 z, i16 damage, u16 count, u16 i);
void msgSendChungusComplete      (int c,  u8 x,  u8 y,  u8 z);
void msgBeingGotHit              (       i16 hp, u8 cause, float knockbackMult, being target, being culprit);
void msgBeingDamage              (int c, i16 hp, u8 cause, float knockbackMult, being target, being culprit, const vec pos);
void msgBeingMove                (being b, vec dpos, vec dvel);
void msgSetTime                  (int c, u32 time);
// 9 = playerSendName
void msgItemDropNew              (int c, const vec pos, const vec vel, const item *itm);
void msgNewGrenade               (const vec pos, const vec rot, float pwr, int cluster, float clusterPwr);
void msgBeamBlast                (const vec pos, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgPlayerMove               (int c, const vec dpos, const vec drot);
void msgPlayerName               (int c, u16 i, const char *name);
// 15 = playerPos ???
// 16 = parseChatMsg ???
// 17 = parseDyingMsg ???
// 18 = chunkData ???
// 19 = setPlayerCount ???
void msgSetPlayerCount           (u16 playerLeaving, u16 playerMax);
void msgPickupItem               (int c, const item itm);
void msgGrenadeExplode           (const vec pos,float pwr, u16 style);
void msgGrenadeUpdate            (int c, const grenade *g, u16 i, u16 count);
void msgFxBeamBlaster            (int c, const vec pa, const vec pb, float beamSize, float damageMultiplier);
void msgFxBeamBlastHit           (int c, const vec pos, u16 size, u16 style);
void msgItemDropUpdate           (int c, const vec pos, const vec vel, const item *itm, u16 i, u16 len, i16 IDPlayer);
void msgPlayerSetData            (int c, i16 hp, u16 activeItem, u32 flags, u16 id);
void msgPlayerSetInventory       (int c, const item *itm, size_t itemCount);
void msgPingPong                 (int c);
void msgAnimalDied               (int c, const animal *a);
void msgPlayerSetEquipment       (int c, const item *itm, size_t itemCount);
void msgItemDropPickup           (int c, uint i);
void msgItemDropBounce           (int c, uint i);
void msgRopeUpdate               (int c, uint i, rope *r);
void msgWaterUpdate              (int c, u16 i, u16 count, u16 x, u16 y, u16 z, i16 strength);
void msgLispSExpr                (int c, const char *str);
void msgLightningStrike          (int c, u16 lx, u16 ly, u16 lz, u16 tx, u16 ty, u16 tz, u16 seed);

const char *networkGetMessageName(uint i);
