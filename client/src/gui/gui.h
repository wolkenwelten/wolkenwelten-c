#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern widget *rootMenu,*widgetGameScreen;

extern uint mousex,mousey,mouseClicked[3];
extern bool mouseHidden;
extern float matOrthoProj[16];

int  getTilesize        ();
void initUI             ();
void resizeUI           ();
void showMouseCursor    ();
void hideMouseCursor    ();
void updateMouse        ();
void drawCursor         ();
void openChat           ();
void renderUI           ();
void guiCancel          ();
void renderLoadingUI    (const char *step);
const char *colorSignalHigh(int err, int warn, int good, int v);
const char *colorSignalLow (int err, int warn, int good, int v);
