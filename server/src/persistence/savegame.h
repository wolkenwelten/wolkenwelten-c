#pragma once
#include "../../../common/src/common.h"

void playerSafeSave        ();
void bigchungusSafeSave    (const bigchungus *c);
void chungusLoad           (chungus *c);
void chungusSave           (chungus *c);
void characterSaveData     (const character *p, const char *pName);
void characterLoadSendData (      character *p, const char *pName, uint c);
void savegameLoad          ();
void savegameSave          ();
