#pragma once
#include "../../../common/src/common.h"
#include "../gfx/textMesh.h"
#include "../game/item.h"

void drawButton(textMesh *m, const char *label, int state, int x, int y, int w, int h);

bool mouseInBox(int x, int y, int w, int h);