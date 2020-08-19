#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/item.h"
#include "../gfx/textMesh.h"


void drawButton(textMesh *m, const char *label, int state, int x, int y, int w, int h);

bool mouseInBox(int x, int y, int w, int h);
