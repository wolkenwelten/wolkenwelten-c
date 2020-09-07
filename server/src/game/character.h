#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/character.h"

extern character characterList[128];
extern int characterCount;

void characterSaveData(int c);
void characterLoadSendData(int c);
