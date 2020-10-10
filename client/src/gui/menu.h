#pragma once
#include "widget.h"

extern int  serverlistCount;
extern char serverlistName[16][32];
extern char serverlistIP[16][64];

void initMenu          ();
void renderMenu        ();
void menuSetError      (char *error);
void menuChangeFocus   (int xoff,int yoff, bool ignoreOnTextInput);
void menuKeyClick      (int btn);
void menuCancel        ();
void menuCloseGame     ();
void startSingleplayer ();
void startMultiplayer  ();
