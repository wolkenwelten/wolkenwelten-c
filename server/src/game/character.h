#pragma once
#include "../../../common/src/common.h"

extern character characterList[128];
extern int characterCount;

character *characterNew       ();
void  characterFree           (character *c);
void  characterInit           (character *c);
bool  characterLOSBlock       (character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock);
void  characterDie            (character *c);
bool  characterHP             (character *c, int addhp);
int   characterGetHP          (character *c);
int   characterGetMaxHP       (character *c);
void  characterAddCooldown    (character *c, int cooldown);
int   characterGetItemAmount  (character *c, uint16_t itemID);
int   characterDecItemAmount  (character *c, uint16_t itemID, int amount);
bool  characterPickupItem     (character *c, uint16_t itemID, int amount);
item *characterGetItemBarSlot (character *c, int i);
void  characterSetActiveItem  (character *c, int i);
void  characterDropItem       (character *c, int i);
uint32_t characterCollision   (character *c, float cx, float cy, float cz,float wd);
