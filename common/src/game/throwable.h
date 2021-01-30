#pragma once
#include "../common.h"

extern throwable throwableList[2048];
extern uint      throwableCount;

void       throwableSendUpdate (int c, uint i);
void       throwableRecvUpdate (const packet *p);

throwable *throwableGetByBeing (being b);
being      throwableGetBeing   (const throwable *g);

throwable *throwableAlloc      ();
void       throwableFree       (throwable *t);

void       throwableNew        (const vec pos, const vec rot, float speed, const item itm, being thrower, u16 flags);
void       throwableUpdateAll  ();
bool       throwableTry        (item *cItem, character *cChar, float strength, uint flags);
bool       throwableTryAim     (item *cItem, character *cChar);
