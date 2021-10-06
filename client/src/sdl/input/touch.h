#pragma once
#include "../../../../common/src/common.h"
#include <SDL.h>

bool touchSneak        ();
bool touchBoost        ();
bool touchPrimary      ();
bool touchSecondary    ();
bool touchTertiary     ();
bool touchThrow        ();
vec  doTouchupdate     (vec vel);
void touchEventHandler (const SDL_Event *e);
