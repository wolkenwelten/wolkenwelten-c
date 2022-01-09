#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"

void          blockTypeInit();

// ToDo: just return the blockType struct directly!
const char   *blockTypeGetName            (blockId b);
int           blockTypeGetHealth          (blockId b);
int           blockTypeGetFireHealth      (blockId b);
int           blockTypeGetFireDamage      (blockId b);
blockCategory blockTypeGetCat             (blockId b);
float         blockTypeGetWeight          (blockId b);
bool          blockTypeValid              (blockId b);
u16           blockTypeGetTexX            (blockId b, side side);
u16           blockTypeGetTexY            (blockId b, side side);
u32           blockTypeGetParticleColor   (blockId b);
mesh         *blockTypeGetMesh            (blockId b);
u16           blockTypeGetIngressMask     (blockId b);
u16           blockTypeGetEgressMask      (blockId b);
