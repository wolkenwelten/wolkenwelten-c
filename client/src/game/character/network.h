#pragma once
#include "../../../../common/src/common.h"
#include "../../../../common/src/game/character.h"

extern int        playerID;
extern character *playerList[32];

void  characterMoveDelta      (character *c, const packet *p);
void  characterDamagePacket   (character *c, const packet *p);
void  characterSetData        (character *c, const packet *p);
void  characterPickupPacket   (character *c, const packet *p);
void  characterUpdatePacket   (              const packet *p);
void  characterSetName        (              const packet *p);
void  characterDyingMessage   (const being victim, const being culprit, deathCause cause);
character *characterGetPlayer (uint i);
char *characterGetPlayerName  (uint i);
int   characterGetPlayerHP    (uint i);
vec   characterGetPlayerDist  (uint i);
void  characterGotHitPacket   (const packet *p);
void  characterRemovePlayer   (int c, int len);
const char *characterGetName  (const character *c);
