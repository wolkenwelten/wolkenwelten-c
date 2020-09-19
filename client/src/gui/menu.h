#pragma once

extern char *menuError;

void initMenu            ();
void renderMenu          ();
void updateMenuClick     (int x, int y, int btn);
void updateMenuGamepad   (int btn);
void changeMenuSelection (int off);
