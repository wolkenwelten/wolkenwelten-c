#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"

void blockTypeGenMeshes();
void blockTypeAddToMesh(u8 b,  mesh *m,const vec pos, const vec size);
