#pragma once
#include "../common.h"

int  fireTick          (chunkOverlay *fire, chunkOverlay *fluid, chunkOverlay *block, int x, int y, int z);
void fireBox           (u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, u8 strength);
void fireBoxExtinguish (u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, u8 strength);
