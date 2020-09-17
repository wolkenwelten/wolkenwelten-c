#pragma once
#include "../common.h"

character *characterNew            ();
void       characterFree           (character *c);
void       characterInit           (character *c);
uint32_t   characterCollision      (const vec pos);
int        characterGetHP          (character *c);
int        characterGetMaxHP       (character *c);
void       characterAddCooldown    (character *c, int cooldown);
void       characterSetPos         (character *c, const vec pos);
void       characterSetRot         (character *c, const vec rot);
void       characterSetVelocity    (character *c, const vec vel);
void       characterAddInaccuracy  (character *c, float inc);
void       characterStartAnimation (character *c, int index, int duration);
bool       characterTryToShoot     (character *c, item *i, int cooldown, int bulletcount);
bool       characterHP             (character *c, int addhp);
bool       characterDamage         (character *c, int hp);
int        characterGetItemAmount  (character *c, uint16_t itemID);
int        characterDecItemAmount  (character *c, uint16_t itemID, int amount);
bool       characterPickupItem     (character *c, uint16_t itemID, int amount);
item      *characterGetItemBarSlot (character *c, int i);
void       characterSetItemBarSlot (character *c, int i, item *itm);
void       characterSetActiveItem  (character *c, int i);
void       characterEmptyInventory (character *c);
void       characterDie            (character *c);
void       characterSwapItemSlots  (character *c, int a,int b);
void       characterMove           (character *c, const vec mov);
void       characterRotate         (character *c, const vec rot);
bool       characterLOSBlock       (character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock);
bool       characterItemReload     (character *c, item *i, int cooldown);
void       characterSetInventoryP  (character *c, const packet *p);
