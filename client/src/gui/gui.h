#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern uint mousex,mousey,mouseClicked[3];
extern bool mouseHidden;
extern float matOrthoProj[16];
extern widget *rootHud;

int  getTilesize     ();
void initUI          ();
void resizeUI        ();
void showMouseCursor ();
void hideMouseCursor ();
void updateMouse     ();
void drawCursor      ();
void openChat        ();
void renderUI        ();
void guiCancel       ();
void renderLoadingUI (const char *step);
