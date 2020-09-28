#pragma once
#include "../../../common/src/common.h"
#include <SDL.h>

bool keyboardSneak        ();
bool keyboardPrimary      ();
bool keyboardSecondary    ();
bool keyboardTertiary     ();
vec  doKeyboardupdate     (vec vel);
void keyboardEventHandler (const SDL_Event *e);
int  keyboardCmdKey       (const SDL_Event *e);
