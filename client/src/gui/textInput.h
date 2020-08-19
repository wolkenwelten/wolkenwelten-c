#pragma once
#include "../../../common/src/common.h"

extern int  textInputLock;
extern bool textInputActive;

bool textInput(int x, int y, int w, int h, int lock);
char *textInputGetBuffer();
void textInputDraw();
void textInputClose();
