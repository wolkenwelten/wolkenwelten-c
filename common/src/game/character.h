#pragma once
#include "../common.h"

character *characterNew            ();
void       characterFree           (      character *c);
void       characterInit           (      character *c);
u32        characterCollision      (const vec pos);
int        characterGetHP          (const character *c);
int        characterGetMaxHP       (const character *c);
void       characterAddCooldown    (      character *c, int cooldown);
void       characterSetPos         (      character *c, const vec pos);
void       characterSetRot         (      character *c, const vec rot);
void       characterSetVelocity    (      character *c, const vec vel);
void       characterAddInaccuracy  (      character *c, float inc);
void       characterStartAnimation (      character *c, int index, int duration);
bool       characterTryToShoot     (      character *c, item *i, int cooldown, int bulletcount);
bool       characterHP             (      character *c, int addhp);
bool       characterDamage         (      character *c, int hp);
int        characterGetItemAmount  (const character *c, u16 itemID);
int        characterDecItemAmount  (      character *c, u16 itemID, int amount);
bool       characterPickupItem     (      character *c, u16 itemID, int amount);
item      *characterGetItemBarSlot (      character *c, uint i);
void       characterSetItemBarSlot (      character *c, uint i, item *itm);
void       characterSwapItemSlots  (      character *c, uint a, uint b);
void       characterSetActiveItem  (      character *c, int i);
void       characterEmptyInventory (      character *c);
void       characterDie            (      character *c);
void       characterMove           (      character *c, const vec mov);
void       characterRotate         (      character *c, const vec rot);
bool       characterLOSBlock       (const character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock);
bool       characterItemReload     (      character *c, item *i, int cooldown);
void       characterSetInventoryP  (      character *c, const packet *p);
