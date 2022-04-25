#pragma once
#include "../common.h"

u8    beingType      (being b);
u32   beingID        (being b);
being beingNew       (u8 type, u32 id);
being beingCharacter (u32 id);
being beingEntity    (u32 id);
being beingProjectile(u32 id);

vec   beingGetPos    (being b);
void  beingSetPos    (being b, const vec pos);
void  beingAddPos    (being b, const vec pos);

vec   beingGetVel    (being b);
void  beingSetVel    (being b, const vec vel);
void  beingAddVel    (being b, const vec vel);

float beingGetWeight (being b);
void  beingDamage    (being b, i16 hp, u8 cause, float knockbackMult, being culprit, const vec pos);
bool   beingAlive     (being b);
const char *beingGetName(being b);

being beingClosest   (const vec pos, float maxDistance);

void  beingListInit       (beingList *bl, beingList *parent);
void  beingListAdd        (beingList *bl, being entry);
void  beingListDel        (beingList *bl, being entry);
beingList *beingListGet   (u16 x, u16 y, u16 z);
beingList *beingListUpdate(beingList *bl, being entry);
being beingListGetClosest (const beingList *bl, const being source, uint type, float *d);

void            beingListEntryInit();
beingListEntry *beingListEntryNew ();
void            beingListEntryFree(beingListEntry *ble);
void            beingListPrint(beingList *bl);
void            beingGetInSphere(vec pos, float r, being source, void (*callback)(vec pos, being b, being source));
