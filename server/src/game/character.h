#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/character.h"

extern character characterList[128];
extern int characterCount;

void characterSaveData    (uint c);
void characterLoadSendData(uint c);
