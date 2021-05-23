#pragma once
#include "../common.h"

void lineFromTo(int x0, int y0, int z0, int x1, int y1, int z1, void (*func)(int x, int y, int z));
