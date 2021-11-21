#pragma once
#include "../common.h"

extern projectile projectileList[8192];

void projectileInit       ();
int  projectileNewID      ();
int  projectileGetClient  (uint i);
bool projectileNew        (const vec pos, const vec rot, being target, being source, uint style, float speed);
bool projectileNewC       (const character *c, being target, uint style);
void projectileUpdateAll  ();
void projectileSendUpdate (uint c, uint i);
void projectileRecvUpdate (uint c, const packet *p);
int  projectileHitCheck   (const vec pos, float mdd, being source);
being projectileGetBeing  (const projectile *p);
