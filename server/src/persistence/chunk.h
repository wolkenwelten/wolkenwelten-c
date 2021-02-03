#pragma once
#include "../../../common/src/common.h"

void       *chunkSave(chunk *c, u8 *buf);
const void *chunkLoad(chungus *c, const u8 *buf);
