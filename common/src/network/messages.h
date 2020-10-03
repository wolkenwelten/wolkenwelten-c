#pragma once
#include "../common.h"

#include "packet.h"
extern packet packetBuffer;

void msgRequestPlayerSpawnPos    ();
void msgPlayerSetPos             (uint c, const vec pos, const vec rot);
void msgRequestChungus           (uint x, uint y, uint z);
void msgDirtyChunk               (uint x, uint y, uint z);
void msgPlaceBlock               (uint x, uint y, uint z, u8 b);
void msgMineBlock                (uint x, uint y, uint z, u8 b);
void msgGoodbye                  ();
void msgBlockMiningUpdate        (uint c, u16 x, u16 y, u16 z, u16 damage, int count, int i);
void msgSendChungusComplete      (uint c, int x, int y, int z);
void msgCharacterGotHit          (uint c, int pwr);
// 9 = playerSendName
void msgItemDropNew              (uint c, const vec pos, const vec vel, const item *itm);
void msgNewGrenade               (const vec pos, const vec rot, float pwr);
void msgBeamBlast                (const vec pos, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void msgPlayerMove               (uint c, const vec dpos, const vec drot);
void msgPlayerName               (uint c, uint i, const char *name);
// 15 = playerPos ???
// 16 = parseChatMsg ???
// 17 = parseDyingMsg ???
// 18 = chunkData ???
// 19 = setPlayerCount ???
void msgSetPlayerCount           (uint playerLeaving, uint playerMax);
void msgPickupItem               (uint c, u16 ID, u16 amount);
void msgGrenadeExplode           (const vec pos,float pwr, int style);
void msgGrenadeUpdate            (uint c, const vec pos, const vec vel, int count, int i);
void msgFxBeamBlaster            (uint c, const vec pa, const vec pb, float beamSize, float damageMultiplier, float recoilMultiplier);
void msgItemDropUpdate           (uint c, const vec pos, const vec vel, u16 i, u16 len, u16 itemID, u16 amount);
void msgPlayerDamage             (uint c, i16 hp, u16 target, u16 cause, u16 culprit);
void msgUnsubChungus             (uint x, uint y, uint z);
void msgPlayerSetData            (uint c, int hp, int activeItem, u32 flags);
void msgPlayerSetInventory       (uint c, const item *itm, size_t itemCount);
