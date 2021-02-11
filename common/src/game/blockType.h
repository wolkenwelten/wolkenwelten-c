#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"

void          blockTypeInit();

const char   *blockTypeGetName            (u8 b);
int           blockTypeGetHP              (u8 b);
int           blockTypeGetFireHP          (u8 b);
int           blockTypeGetFireDmg         (u8 b);
blockCategory blockTypeGetCat             (u8 b);
bool          blockTypeValid              (u8 b);
u16           blockTypeGetTexX            (u8 b, side side);
u16           blockTypeGetTexY            (u8 b, side side);
u32           blockTypeGetParticleColor   (u8 b);
mesh         *blockTypeGetMesh            (u8 b);
