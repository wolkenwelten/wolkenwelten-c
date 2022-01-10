#pragma once
#include "../../../common/src/common.h"

extern int chunkOverlayAllocated;
extern int chunkOverlayUsed;

chunkOverlay *chunkOverlayAllocate();
void chunkOverlayFree(chunkOverlay *d);
