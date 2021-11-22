#pragma once
#include "../../../../common/src/common.h"
#include "../widget.h"

void widgetAddPopup   (const widget *wid, box2D area);
void widgetDrawSingle (const widget *wid, textMesh *mesh, const box2D area);
void widgetDrawPopups (textMesh *mesh);
