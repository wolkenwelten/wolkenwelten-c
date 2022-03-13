#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

void     entityDraw      (const entity *e);
void     entityDrawAll   ();
vec      entityScreenPos (const entity *e);
