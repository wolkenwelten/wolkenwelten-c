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
void  characterDraw            (const character *c);
void  characterHitCheck        (      character *c, int origin, const vec pos, const vec rot, int pwr);
void  characterDropItem        (      character *c, int i);
void  characterMoveDelta       (      character *c, const packet *p);
void  characterDamagePacket    (      character *c, const packet *p);
void  characterSetData         (      character *c, const packet *p);
void  characterSetPlayerPos    (                    const packet *p);
void  characterGotHitBroadcast (int c, int pwr);
void  characterRemovePlayer    (int c, int len);

void  charactersUpdate         ();
void  characterDrawAll         ();

bool  itemPlaceBlock           (item *i,character *chr, int to);
