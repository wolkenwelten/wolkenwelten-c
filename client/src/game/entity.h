#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

void     entityDrawAll   ();
vec      entityScreenPos (const entity *e);
void     entityUpdateFromServer(const packet *p);
void     entityDeleteFromServer(const packet *p);
