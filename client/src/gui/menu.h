#pragma once

typedef struct widget widget;

extern char *menuError;
extern widget *menuCurSelection;

void initMenu            ();
void renderMenu          ();
void menuChangeFocus     (int xoff,int yoff);
void menuKeyClick        (int btn);
void changeMenuSelection (int off);
