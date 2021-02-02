#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/nujel/nujel.h"
#include "widget.h"

extern widget *lispPanel,*lispLog;
extern bool    lispPanelVisible;
extern int     lispInputCheckCountdown;
extern lSymbol lispAutoCompleteList[32];
extern uint    lispAutoCompleteLen;
extern int     lispAutoCompleteStart;
extern int     lispAutoCompleteEnd;
extern int     lispAutoCompleteSelection;

void lispPanelOpen              ();
void lispPanelClose             ();
void lispPanelToggle            ();
void lispInputInit              ();
void lispPanelShowReply         (lVal *sym, const char *reply);
void lispPanelCheckAutoComplete ();
