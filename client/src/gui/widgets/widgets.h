#pragma once
#include "../../../../common/src/common.h"
#include "../widget.h"

void widgetAddPopup   (const widget *wid, uint x, uint y, uint w, uint h);
void widgetDrawSingle (const widget *wid, textMesh *mesh, int x, int y, int w, int h);
void widgetDrawPopups (textMesh *mesh);
