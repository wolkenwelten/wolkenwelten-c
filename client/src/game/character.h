#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/character.h"

extern character *player;

void  characterPrimary         (      character *c);
void  characterSecondary       (      character *c);
void  characterTertiary        (      character *c);
void  characterStopMining      (      character *c);
float characterMineProgress    (      character *c);
void  characterUpdate          (      character *c);
void  characterFireHook        (      character *c);
void  characterFreeHook        (      character *c);
void  characterDraw            (      character *c);
void  characterDropItem        (      character *c, int i);
bool  characterPlaceBlock      (      character *c, item *i);
void  characterMoveDelta       (      character *c, const packet *p);
void  characterDamagePacket    (      character *c, const packet *p);
void  characterSetData         (      character *c, const packet *p);
void  characterUpdatePacket    (                    const packet *p);
void  characterSetName         (                    const packet *p);

character *characterGetPlayer  (uint i);
char *characterGetPlayerName   (uint i);
int   characterGetPlayerHP     (uint i);
vec   characterGetPlayerDist   (uint i);
void  characterGotHitBroadcast (int c, int pwr);
void  characterRemovePlayer    (int c, int len);
int   characterHitCheck        (const vec pos, float mdd, int damage, int cause, uint iteration);

void  charactersUpdate         ();
void  characterDrawAll         ();
