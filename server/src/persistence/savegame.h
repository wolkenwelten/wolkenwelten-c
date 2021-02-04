#pragma once
#include "../../../common/src/common.h"

typedef enum {
	saveTypeUndefined=0,
	saveTypeChunk,
	saveTypeItemDrop,
	saveTypeAnimal,
	saveTypeFire,
	saveTypeThrowable
} saveType;

void playerSafeSave        ();
void bigchungusSafeSave    (const bigchungus *c);
void chungusLoad           (chungus *c);
void chungusSave           (chungus *c);
void savegameLoad          ();
void savegameSave          ();
