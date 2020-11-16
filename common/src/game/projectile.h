#pragma once
#include "../common.h"

extern projectile projectileList[8192];

int  projectileNewID     ();
int  projectileGetClient (uint i);
void projectileNew       (const vec pos, const vec rot, being target, being source, uint style);
void projectileNewC      (const character *c, being target, uint style);
void projectileUpdateAll ();
void projectileSendUpdate(uint c, uint i);
void projectileRecvUpdate(uint c, const packet *p);
int  projectileHitCheck  (const vec pos, float mdd, int dmg, int cause, u16 iteration, being source);
being projectileGetBeing (const projectile *p);
