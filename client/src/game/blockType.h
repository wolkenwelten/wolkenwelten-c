#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"

void blockTypeGenMeshes();
void blockTypeAddToMesh(blockId b, mesh *m, const vec pos, const vec size);
void blockTypeDraw     (blockId b, vec pos, float alpha, int depthOffset);
