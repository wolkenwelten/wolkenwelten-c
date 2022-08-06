#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern const lSymbol *lispAutoCompleteList[32];
extern widget  *lispPanel,*lispLog;
extern bool     lispPanelVisible;
extern int      lispInputCheckCountdown;
extern char     lispAutoCompleteDescription[256];
extern uint     lispAutoCompleteLen;
extern int      lispAutoCompleteStart;
extern int      lispAutoCompleteEnd;
extern int      lispAutoCompleteSelection;
extern bool     lispAutoCompleteCompleteSymbol;

void lispPanelOpen              ();
void lispPanelClose             ();
void lispPanelToggle            ();
void initLispPanel              ();
void lispPanelShowReply         (const char *reply);
void lispPanelCheckAutoComplete ();
void lispPanelCheckAutoCompleteDescription();
