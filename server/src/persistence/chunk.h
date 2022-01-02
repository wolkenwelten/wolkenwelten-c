#pragma once
#include "../../../common/src/common.h"
#include "savegame.h"

void       *chunkSave(chunk *c, void *buf, saveType t);
const void *chunkLoad(chungus *c, const void *buf);
