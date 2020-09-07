#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/character.h"

extern character *player;

void  characterPrimary        (character *c);
void  characterSecondary      (character *c);
void  characterTertiary       (character *c);
void  characterStopMining     (character *c);
float characterMineProgress   (character *c);
void  characterUpdate         (character *c);
void  charactersUpdate        ();
void  characterFireHook       (character *c);
void  characterFreeHook       (character *c);
void  characterDraw           (character *c);
void  characterDrawAll        ();
void  characterHitCheck       (character *c, int origin, float x, float y, float z, float yaw, float pitch, float roll, int pwr);
void  characterGotHitBroadcast(int c,int pwr);
void  characterDropItem       (character *c, int i);
void  characterMoveDelta      (character *c, const packet *p);
void  characterDamagePacket   (character *c, const packet *p);
void  characterSetData        (character *c, const packet *p);
void  characterSetPlayerPos   (const packet *p);
void  characterRemovePlayer   (int c, int len);

bool  itemPlaceBlock          (item *i, character *chr, int to);
