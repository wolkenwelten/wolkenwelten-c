#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern int  textInputBufferLen;
extern int  textInputCursorPos;
extern int  textInputMark;

void  textInputFocus  (widget *wid);
void  textInputBlur   (widget *wid);
int   textInputActive ();
void  textInputEnter  ();
