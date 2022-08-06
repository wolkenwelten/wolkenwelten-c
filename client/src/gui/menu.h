#pragma once
#include "widget.h"

extern widget *rootMenu;

extern widget *menuBackground, *menuAttribution;

void menuSetError      (const char *error);
void menuChangeFocus   (int xoff,int yoff, bool ignoreOnTextInput);
void menuKeyClick      (int btn);
void serverListAdd     (const char *address, const char *name);
bool menuCancel        ();
void menuCloseGame     ();

void initAttribution   ();
void openAttributions  ();
