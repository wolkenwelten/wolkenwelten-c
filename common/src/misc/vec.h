#pragma once
#include "../gfx_structs.h"
#include "../../nujel/lib/misc/vec.h"

int   vecInWorld  (const vec a);
u64   vecToPacked (const vec v);
vec   packedToVec (const u64 v);
