#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/character.h"

void characterDmgPacket(uint c, const packet *p);
int  characterHitCheck (const vec pos, float mdd, int damage, int cause, u16 iteration);
