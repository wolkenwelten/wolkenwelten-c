#pragma once
#include "../../../common/src/common.h"
#include "../gfx/textMesh.h"

void     commitOverlayColor();
uint32_t getOverlayColor();
void     setOverlayColor(uint32_t color, uint32_t animationDuration);
void     resetOverlayColor();
void     drawOverlay(textMesh *m);