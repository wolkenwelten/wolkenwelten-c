#pragma once
#include "../../../common/src/common.h"

void          blockTypeInit();

const char   *blockTypeGetName         (u8 b);
int           blockTypeGetHP           (u8 b);
blockCategory blockTypeGetCat          (u8 b);
bool          blockTypeValid           (u8 b);
u16           blockTypeGetTexX         (u8 b, int side);
u16           blockTypeGetTexY         (u8 b, int side);
u32           blockTypeGetParticleColor(u8 b);
mesh         *blockTypeGetMesh         (u8 b);
