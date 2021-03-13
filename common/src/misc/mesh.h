#pragma once
#include "../common.h"

extern mesh *meshPear;
extern mesh *meshBunny;
extern mesh *meshHook;

mesh *meshGet    (uint i);
uint  meshIndex  (const mesh *m);
