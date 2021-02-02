#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/nujel/nujel.h"
#include "widget.h"

extern widget *lispPanel,*lispLog;
extern bool    lispPanelVisible;

void lispPanelOpen      ();
void lispPanelClose     ();
void lispPanelToggle    ();
void lispInputInit      ();
void lispPanelShowReply (lVal *sym, const char *reply);
