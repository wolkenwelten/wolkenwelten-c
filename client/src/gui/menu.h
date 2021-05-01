#pragma once
#include "widget.h"

extern widget *rootMenu;
extern widget *menuErrorLabel;

extern int  serverlistCount;
extern char serverlistName[16][32];
extern char serverlistIP[16][64];

void initMenu          ();
void menuSetError      (const char *error);
void menuChangeFocus   (int xoff,int yoff, bool ignoreOnTextInput);
void menuKeyClick      (int btn);
void serverListAdd     (const char *address, const char *name);
bool menuCancel        ();
void menuCloseGame     ();
void startSingleplayer ();
void startMultiplayer  ();
void closeAllMenus     ();
void openAttributions  ();
