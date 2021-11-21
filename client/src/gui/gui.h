#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern widget *rootMenu,*widgetGameScreen;

extern uint mousex,mousey,mouseClicked[3];
extern bool mouseHidden;
extern float matOrthoProj[16];

void handlerRoot        (widget *wid);
void handlerRootHud     (widget *wid);
int  getTilesize        ();
void initGUI            ();
void closeAllMenus      ();
void resizeUI           ();
void showMouseCursor    ();
void hideMouseCursor    ();
void updateMouse        ();
void drawMenuBackground ();
void drawCursor         ();
void renderUI           ();
bool guiCancel          ();
void guiEscape          ();
void renderLoadingUI    (const char *step);
const char *colorSignalHigh(int err, int warn, int good, int v);
const char *colorSignalLow (int err, int warn, int good, int v);
