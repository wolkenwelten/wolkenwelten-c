#pragma once
#include "widget.h"

void initMenu            ();
void renderMenu          ();
void menuSetError        (char *error);
void menuChangeFocus     (int xoff,int yoff);
void menuKeyClick        (int btn);
void menuCancel          ();
void menuCloseGame       ();
