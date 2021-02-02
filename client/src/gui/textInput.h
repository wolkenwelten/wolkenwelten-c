#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern int  textInputCursorPos;
extern int  textInputMark;
extern uint textInputLastEvent;

void  textInputFocus  (widget *wid);
void  textInputBlur   (widget *wid);
int   textInputActive ();
void  textInputEnter  ();
