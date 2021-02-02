#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/nujel/nujel.h"
#include "widget.h"

extern widget *lispPanel;
extern widget *lispLog;
extern bool    lispPanelVisible;

void openLispPanel      ();
void closeLispPanel     ();
void toggleLispPanel    ();
void lispInputInit      ();
void lispPanelShowReply (lVal *sym, const char *reply);
