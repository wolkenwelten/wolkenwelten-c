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

void       characterAddCooldown    (      character *c, int cooldown);
void       characterUpdateEquipment(      character *c);
void       characterSetPos         (      character *c, const vec pos);
void       characterSetRot         (      character *c, const vec rot);
void       characterSetVelocity    (      character *c, const vec vel);
void       characterSetInaccuracy  (      character *c, float inacc);
void       characterAddInaccuracy  (      character *c, float inc);
void       characterAddRecoil      (      character *c, float recoil);
void       characterStopAnimation  (      character *c);
void       characterStartAnimation (      character *c, animType index, int duration);
bool       characterTryToShoot     (      character *c, item *i, int cooldown, int bulletcount);
bool       characterTryToUse       (      character *c, item *i, int cooldown, int itemcount);
bool       characterHP             (      character *c, int addhp);
bool       characterDamage         (      character *c, int hp);
int        characterGetItemAmount  (const character *c, u16 itemID);
int        characterDecItemAmount  (      character *c, u16 itemID, int amount);
int        characterPickupItem     (      character *c, u16 itemID, int amount);
bool       characterPlaceBlock     (      character *c, item *i);
item      *characterGetItemBarSlot (      character *c, uint i);
item      *characterGetActiveItem  (      character *c);
void       characterSetItemBarSlot (      character *c, uint i, item *itm);
void       characterSwapItemSlots  (      character *c, uint a, uint b);
void       characterSetActiveItem  (      character *c, int i);
void       characterEmptyInventory (      character *c);
void       characterDie            (      character *c);
void       characterMove           (      character *c, const vec mov);
void       characterRotate         (      character *c, const vec rot);
ivec       characterLOSBlock       (const character *c, bool returnBeforeBlock);
bool       characterItemReload     (      character *c, item *i, int cooldown);
void       characterSetInventoryP  (      character *c, const packet *p);
void       characterSetEquipmentP  (      character *c, const packet *p);

character *characterGetByBeing     (being b);
being      characterGetBeing       (const character *c);

float      characterGetMaxHookLen  (const character *c);
float      characterGetHookWinchS  (const character *c);
character *characterClosest        (const vec pos, float maxDistance);

int   characterHitCheck       (const vec pos, float mdd, int damage, int cause, u16 iteration, being source);
