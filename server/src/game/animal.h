#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

void        animalSyncPlayer       (u8 c);
void        animalDelChungus       (const chungus *c);
void        animalIntroChungus     (u8 c, const chungus *chng);
void        animalUpdatePriorities (u8 c);
