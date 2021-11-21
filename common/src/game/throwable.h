#pragma once
#include "../common.h"

extern throwable throwableList[2048];
extern uint      throwableCount;

void       throwableInit        ();
void       throwableSendUpdate  (int c, uint i);
void       throwableEmptyUpdate (int c);
void       throwableRecvUpdate  (const packet *p);

throwable *throwableGetByBeing  (being b);
being      throwableGetBeing    (const throwable *g);

throwable *throwableAlloc       ();
void       throwableFree        (throwable *t);
void       throwableDel         (uint i);

void       throwableNew         (const vec pos, const vec rot, float speed, const item itm, being thrower, i8 damage, u8 flags);
void       throwableUpdateAll   ();
bool       throwableTry         (item *cItem, character *cChar, float strength, int dmg, uint flags);
bool       throwableTryAim      (item *cItem, character *cChar);
void       throwableDelChungus  (chungus *c);
