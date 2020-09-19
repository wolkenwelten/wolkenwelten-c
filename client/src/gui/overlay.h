#pragma once
#include "../../../common/src/common.h"
#include "../gfx/textMesh.h"

void commitOverlayColor ();
u32  getOverlayColor    ();
void setOverlayColor    (u32 color, u32 animationDuration);
void resetOverlayColor  ();
void drawOverlay        (textMesh *m);
