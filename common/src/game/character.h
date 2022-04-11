#pragma once
#include "../common.h"

extern character  characterList[64];
extern uint       characterCount;

character *characterNew            ();
void       characterFree           (      character *c);
void       characterInit           (      character *c);
u32        characterCollision      (const vec pos);
u8         characterCollisionBlock (const vec pos, vec *retPos);
vec        characterGetCollisionVec(const vec pos);
int        characterGetHP          (const character *c);
int        characterGetMaxHP       (const character *c);

void       characterOpenGlider     (      character *c);
void       characterCloseGlider    (      character *c);
void       characterToggleGlider   (      character *c);

void       characterOpenConsMode   (      character *c);
void       characterCloseConsMode  (      character *c);
void       characterToggleConsMode (      character *c);

bool       characterIsAiming       (const character *c);
bool       characterIsThrowAiming  (const character *c);
void       characterToggleAim      (      character *c, float zoom);
void       characterToggleThrowAim (      character *c, float zoom);
void       characterStopAim        (      character *c);

void       characterSetCooldown    (      character *c, int cooldown);
void       characterSetPos         (      character *c, const vec pos);
void       characterSetRot         (      character *c, const vec rot);
void       characterSetVelocity    (      character *c, const vec vel);
void       characterSetInaccuracy  (      character *c, float inacc);
void       characterAddInaccuracy  (      character *c, float inc);
void       characterAddRecoil      (      character *c, float recoil);
void       characterStopAnimation  (      character *c);
void       characterStartAnimation (      character *c, animType index, int duration);
bool       characterHP             (      character *c, int addhp);
bool       characterCheckHealth    (      character *c);
bool       characterDamage         (      character *c, int hp);
void       characterDie            (      character *c);
void       characterMove           (      character *c, const vec mov);
void       characterRotate         (      character *c, const vec rot);
vec        characterLOSBlock       (const character *c, bool returnBeforeBlock);
bool       characterPlaceBlock     (      character *c, blockId b);

character *characterGetByBeing     (being b);
being      characterGetBeing       (const character *c);

float      characterGetMaxHookLen  (const character *c);
float      characterGetHookWinchS  (const character *c);
character *characterClosest        (const vec pos, float maxDistance);

int         characterHitCheck      (const vec pos, float mdd, int damage, int cause, u16 iteration, being source);
const char *characterGetName       (const character *c);
void        characterUpdateItems   (const character *c);
