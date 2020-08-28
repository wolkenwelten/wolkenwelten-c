#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/character.h"

extern character *player;
extern float cdrag;
extern float clift;

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
void  characterHitCheck       (character *c, int origin, float x, float y, float z, float yaw, float pitch, float roll, float pwr);
void  characterGotHitBroadcast(int c,float pwr);
void  characterDropItem       (character *c, int i);

void  characterMoveDelta      (character *c, packet *p);
void  characterDamagePacket   (character *c, packet *p);
void  characterSetPlayerPos   (const packet *p);
void  characterRemovePlayer   (int c, int len);

bool  itemPlaceBlock          (item *i, character *chr, int to);
